/**
 * @file app.cpp
 * @brief Main application implementation.
 *
 * Implements the App class that orchestrates the Workbench TUI layout
 * and initializes the background monitoring system. Creates a grid
 * layout with system resources, models, and settings panels.
 */

#include "app.h"
#include "configManager.h"
#include "modelsPanel.h"
#include "serverInfoPanel.h"
#include "settingsPanel.h"
#include "systemResourcesPanel.h"
#include "terminalPanel.h"
#include "terminalPresetsPanel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace ftxui;

void App::run()
{
	auto screen = ScreenInteractive::Fullscreen();
	TerminalPanel terminalPanel(screen);
	SettingsPanel settingsPanel;
	ModelsPanel modelsPanel;

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

	auto serverContent = Renderer([] {
		return hbox({ text("Some really long status about the server probably "
						   "here."),
					  filler(),
					  ServerInfoPanel::render() });
	});

	auto terminalContent = terminalPanel.component();

	// Placeholder for when we implement reading live log outputs from llama.cpp
	auto logOutputContent =
		Renderer([] { return window(text(""), text(""), ftxui::EMPTY) | flex; });

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
	auto &config = ConfigManager::instance().getConfig();
	for (const auto &preset : config.terminalPresets) {
		auto panel =
			std::make_unique<TerminalPanel>(screen, preset.initialCommand);
		auto component = panel->component();
		tabValues.push_back(preset.name);
		tabContainer->Add(component);
		int idx = static_cast<int>(tabValues.size()) - 1;
		dynamicTerminals.push_back({ std::move(panel), idx, preset.name });
	}

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

		return vbox({ SystemResourcesPanel::render(),
					  separatorCharacter("*") | bold | color(Color::Orange3),
					  panel,
					  serverContent->Render() }) |
			   flex;
	});

	// When a terminal tab is active, intercept keyboard events before
	// the Toggle component consumes them (e.g. arrow keys, Tab, chars).
	auto root =
		container | CatchEvent([&](Event event) {
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
}
