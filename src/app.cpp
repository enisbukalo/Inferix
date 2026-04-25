/**
 * @file app.cpp
 * @brief Main application implementation.
 *
 * Implements the App class that orchestrates the Workbench TUI layout
 * and initializes the background monitoring system. Creates a grid
 * layout with system resources, models, and settings panels.
 */

#include "app.h"
#include "appDependencies.h"
#include "configManager.h"
#include "cpuMonitor.h"
#include "gpuMonitor.h"
#include "llamaServerProcess.h"
#include "modelInfoMonitor.h"
#include "modelsIni.h"
#include "modelsPanel.h"
#include "ramMonitor.h"
#include "serverInfoPanel.h"
#include "serverLogPanel.h"
#include "settingsPanel.h"
#include "systemMonitorRunner.h"
#include "systemResourcesPanel.h"
#include "terminalPanel.h"
#include "terminalPresetsPanel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace ftxui;

void App::run()
{
	spdlog::info("App::run() - initializing");

	// Clear llama-server log on startup (truncate existing file)
	{
		std::string logPath = ConfigManager::getLogsDir() + "/llama-server.log";
		std::ofstream(logPath, std::ios::trunc).close();
	}

	auto screen = ScreenInteractive::Fullscreen();

	// Disable FTXUI's forced Ctrl-C handling. By default FTXUI generates
	// SIGABRT for Ctrl-C if unhandled. We intercept it ourselves and forward
	// the ETX byte (\x03) to the PTY so the shell process gets SIGINT.
	screen.ForceHandleCtrlC(false);

	// Load config and start the system monitor singleton with the screen
	// reference This allows it to trigger redraws when monitor data updates
	auto &config = ConfigManager::instance().getConfig();
	SystemMonitorRunner::instance().start(&screen, config.ui.refreshRateMs);

	// Build dependency injection struct from singleton instances
	AppDependencies deps{
		ConfigManager::instance(),		// IConfigManager&
		LlamaServerProcess::instance(), // ILlamaServerProcess&
		ModelInfoMonitor::instance(),	// IModelInfoMonitor&
		ModelsIni::instance(),			// IModelsIni&
		CpuMonitor::instance(),			// ICpuMonitor&
		MemoryMonitor::instance(),		// IMemoryMonitor&
		GpuMonitor::instance()			// IGpuMonitor&
	};

	TerminalPanel terminalPanel(screen);
	SettingsPanel settingsPanel(deps);
	ModelsPanel modelsPanel(deps);
	ServerLogPanel serverLogPanel(screen);

	std::vector<std::string> tabValues{ "App Settings",
										"Model Settings",
										"Server Log",
										"Terminal" };
	int selectedTab = 0;
	auto tabToggle = Toggle(&tabValues, &selectedTab);

	// Settings tab - interactive configuration components + terminal presets
	auto settingsInner = Container::Vertical({
		settingsPanel.component(),
	});
	auto settingsContent = Renderer(settingsInner, [&] {
		return window(text(""), flex(settingsInner->Render()), ftxui::EMPTY) |
			   flex;
	});

	// Model tab - interactive configuration components
	auto modelInner = Container::Vertical({
		modelsPanel.component(),
	});
	auto modelContent = Renderer(modelInner, [&] {
		return window(text(""), flex(modelInner->Render()), ftxui::EMPTY) | flex;
	});

	auto serverContent = Renderer([&] {
		return hbox({ text("Some really long status about the server probably "
						   "here."),
					  filler(),
					  ServerInfoPanel::render(deps.server) });
	});

	auto terminalContent = terminalPanel.component();

	// Server Log tab - shows live output from llama-server
	auto logOutputContent = serverLogPanel.component();

	// Spawn ServerLogPanel terminal (watching the log file)
	serverLogPanel.start();

	auto tabContainer = Container::Tab(
		{ settingsContent, modelContent, logOutputContent, terminalContent },
		&selectedTab);

	// Dynamically added tabs - loaded from config.
	// Store dynamic terminal panels so they survive until the event loop ends.
	struct DynamicTerminal
	{
		std::unique_ptr<TerminalPanel> panel;
		int tabIndex;
		std::string presetName; // Track which preset this is
	};
	std::vector<DynamicTerminal> dynamicTerminals;

	// Load terminals from config
	// (config already loaded above for SystemMonitorRunner)
	for (const auto &preset : config.terminalPresets) {
		auto panel =
			std::make_unique<TerminalPanel>(screen, preset.initialCommand);
		auto component = panel->component();
		tabValues.push_back(preset.name);
		tabContainer->Add(component);
		int idx = static_cast<int>(tabValues.size()) - 1;
		dynamicTerminals.push_back({ std::move(panel), idx, preset.name });
	}

	spdlog::info("Spawned {} terminal preset(s)", dynamicTerminals.size());

	auto interactive = Container::Vertical({ tabToggle, tabContainer }) | flex;

	// Spawn all terminals eagerly so they're ready when the user switches tabs.
	terminalPanel.spawn();
	for (auto &dt : dynamicTerminals) {
		dt.panel->spawn();
	}

	int prevTab = selectedTab;

	auto container = Renderer(interactive, [&] {
		// Auto-capture when switching to a terminal tab.
		if (selectedTab != prevTab) {
			prevTab = selectedTab;
			if (selectedTab == 3) {
				terminalPanel.setCapturing(true);
			}
			for (auto &dt : dynamicTerminals) {
				if (selectedTab == dt.tabIndex) {
					dt.panel->setCapturing(true);
				}
			}
		}

		// Check if any active terminal tab is capturing input.
		bool anyCapturing = false;
		if (selectedTab == 3 && terminalPanel.isCapturing()) {
			anyCapturing = true;
		}
		for (auto &dt : dynamicTerminals) {
			if (selectedTab == dt.tabIndex && dt.panel->isCapturing()) {
				anyCapturing = true;
			}
		}

		auto panel = interactive->Render() | borderRounded;
		if (anyCapturing)
			panel = panel | color(Color::LightGreen);

		return vbox({ SystemResourcesPanel::render(deps.cpu,
												   deps.mem,
												   deps.gpu,
												   deps.modelInfo),
					  separatorCharacter("*") | bold | color(Color::Orange3),
					  panel,
					  serverContent->Render() }) |
			   flex;
	});

	// When a terminal tab is active, intercept keyboard events before
	// the Toggle component consumes them (e.g. arrow keys, Tab, chars).
	auto root =
		container | CatchEvent([&](Event event) {
			// Ctrl+C — forward ETX byte to the active terminal's PTY so the
			// shell's line discipline generates SIGINT for the foreground
			// process group. This prevents FTXUI from quitting the app.
			if (event == Event::CtrlC) {
				if (selectedTab == 3 && terminalPanel.isCapturing()) {
					terminalPanel.sendCtrlC();
					return true;
				}
				for (auto &dt : dynamicTerminals) {
					if (selectedTab == dt.tabIndex && dt.panel->isCapturing()) {
						dt.panel->sendCtrlC();
						return true;
					}
				}
				return false;
			}

			if (selectedTab == 3 && terminalPanel.wantsEvent(event)) {
				return terminalPanel.handleEvent(event);
			}
			for (auto &dt : dynamicTerminals) {
				if (selectedTab == dt.tabIndex && dt.panel->wantsEvent(event)) {
					return dt.panel->handleEvent(event);
				}
			}
			return false;
		});

	screen.Loop(root);

	spdlog::info("App::run() - exiting");
}
