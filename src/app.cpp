/**
 * @file app.cpp
 * @brief Main application implementation.
 *
 * Implements the App class that orchestrates the Inferix TUI layout
 * and initializes the background monitoring system. Creates a grid
 * layout with system resources, models, and settings panels.
 */

#include "app.h"
#include "inference_settings_panel.h"
#include "load_settings_panel.h"
#include "model_presets_panel.h"
#include "models_panel.h"
#include "server_info_panel.h"
#include "system_monitor_runner.h"
#include "system_resources_panel.h"
#include "terminal_panel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace ftxui;

void App::Run()
{
	auto screen = ScreenInteractive::Fullscreen();
	SystemMonitorRunner runner(screen);
	TerminalPanel terminalPanel(screen);

	std::vector<std::string> tabValues{ "Settings", "Server Log", "Terminal" };
	int selectedTab = 0;
	auto tabToggle = Toggle(&tabValues, &selectedTab);

	auto settingsContent = Renderer([] {
		return window(text(""),
					  hbox({
						  vbox({ ModelsPanel::Render(),
								 ModelPresetsPanel::Render() }) |
							  flex,
						  filler(),
						  // separatorEmpty(),
						  vbox({ LoadSettingsPanel::Render(),
								 InferenceSettingsPanel::Render() }) |
							  flex,
					  }),
					  ftxui::EMPTY) |
			   flex;
	});

	auto serverContent = Renderer([] {
		return hbox({ text("Some really long status about the server probably "
						   "here."),
					  filler(),
					  ServerInfoPanel::Render() });
	});

	auto terminalContent = terminalPanel.Component();

	// Placeholder for when we implement reading live log outputs from llama.cpp
	auto logOutputContent =
		Renderer([] { return window(text(""), text(""), ftxui::EMPTY) | flex; });

	auto tabContainer =
		Container::Tab({ settingsContent, logOutputContent, terminalContent },
					   &selectedTab);

	// Dynamically added tabs (simulating loading from a config file).
	// Store dynamic terminal panels so they survive until the event loop ends.
	struct DynamicTerminal
	{
		std::unique_ptr<TerminalPanel> panel;
		int tabIndex;
	};
	std::vector<DynamicTerminal> dynamicTerminals;

	{
		auto panel = std::make_unique<TerminalPanel>(screen, "opencode");
		auto component = panel->Component();
		tabValues.push_back("Opencode");
		tabContainer->Add(component);
		int idx = static_cast<int>(tabValues.size()) - 1;
		dynamicTerminals.push_back({ std::move(panel), idx });
	}

	{
		auto panel = std::make_unique<TerminalPanel>(screen, "gitui");
		auto component = panel->Component();
		tabValues.push_back("GitUI");
		tabContainer->Add(component);
		int idx = static_cast<int>(tabValues.size()) - 1;
		dynamicTerminals.push_back({ std::move(panel), idx });
	}

	auto interactive = Container::Vertical({ tabToggle, tabContainer }) | flex;

	// Spawn all terminals eagerly so they're ready when the user switches tabs.
	terminalPanel.Spawn();
	for (auto &dt : dynamicTerminals) {
		dt.panel->Spawn();
	}

	int prevTab = selectedTab;

	auto container = Renderer(interactive, [&] {
		// Auto-capture when switching to a terminal tab.
		if (selectedTab != prevTab) {
			prevTab = selectedTab;
			if (selectedTab == 2) {
				terminalPanel.SetCapturing(true);
			}
			for (auto &dt : dynamicTerminals) {
				if (selectedTab == dt.tabIndex) {
					dt.panel->SetCapturing(true);
				}
			}
		}

		// Check if any active terminal tab is capturing input.
		bool anyCapturing = false;
		if (selectedTab == 2 && terminalPanel.IsCapturing()) {
			anyCapturing = true;
		}
		for (auto &dt : dynamicTerminals) {
			if (selectedTab == dt.tabIndex && dt.panel->IsCapturing()) {
				anyCapturing = true;
			}
		}

		auto panel = interactive->Render() | borderRounded;
		if (anyCapturing)
			panel = panel | color(Color::LightGreen);

		return vbox({ SystemResourcesPanel::Render(),
					  separatorCharacter("*") | bold | color(Color::Orange3),
					  panel,
					  serverContent->Render() }) |
			   flex;
	});

	// When a terminal tab is active, intercept keyboard events before
	// the Toggle component consumes them (e.g. arrow keys, Tab, chars).
	auto root =
		container | CatchEvent([&](Event event) {
			if (selectedTab == 2 && terminalPanel.WantsEvent(event)) {
				return terminalPanel.HandleEvent(event);
			}
			for (auto &dt : dynamicTerminals) {
				if (selectedTab == dt.tabIndex && dt.panel->WantsEvent(event)) {
					return dt.panel->HandleEvent(event);
				}
			}
			return false;
		});

	screen.Loop(root);
}
