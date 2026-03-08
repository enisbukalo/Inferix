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
	TerminalPanel terminal_panel(screen);

	std::vector<std::string> tab_values{ "Settings", "Server Log", "Terminal" };
	int selected_tab = 0;
	auto tab_toggle = Toggle(&tab_values, &selected_tab);

	auto settings_content = Renderer([] {
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

	auto server_content = Renderer([] {
		return hbox({ text("Some really long status about the server probably "
						   "here."),
					  filler(),
					  ServerInfoPanel::Render() });
	});

	auto terminal_content = terminal_panel.Component();

	// Placeholder for when we implement reading live log outputs from llama.cpp
	auto log_output_content =
		Renderer([] { return window(text(""), text(""), ftxui::EMPTY) | flex; });

	auto tab_container = Container::Tab(
		{ settings_content, log_output_content, terminal_content },
		&selected_tab);

	// Dynamically added tabs (simulating loading from a config file).
	// Store dynamic terminal panels so they survive until the event loop ends.
	struct DynamicTerminal
	{
		std::unique_ptr<TerminalPanel> panel;
		int tab_index;
	};
	std::vector<DynamicTerminal> dynamic_terminals;

	{
		auto panel = std::make_unique<TerminalPanel>(screen, "opencode");
		auto component = panel->Component();
		tab_values.push_back("Opencode");
		tab_container->Add(component);
		int idx = static_cast<int>(tab_values.size()) - 1;
		dynamic_terminals.push_back({ std::move(panel), idx });
	}

	{
		auto panel = std::make_unique<TerminalPanel>(screen, "gitui");
		auto component = panel->Component();
		tab_values.push_back("GitUI");
		tab_container->Add(component);
		int idx = static_cast<int>(tab_values.size()) - 1;
		dynamic_terminals.push_back({ std::move(panel), idx });
	}

	auto interactive = Container::Vertical({ tab_toggle, tab_container }) | flex;

	// Spawn all terminals eagerly so they're ready when the user switches tabs.
	terminal_panel.Spawn();
	for (auto &dt : dynamic_terminals) {
		dt.panel->Spawn();
	}

	int prev_tab = selected_tab;

	auto container = Renderer(interactive, [&] {
		// Auto-capture when switching to a terminal tab.
		if (selected_tab != prev_tab) {
			prev_tab = selected_tab;
			if (selected_tab == 2) {
				terminal_panel.SetCapturing(true);
			}
			for (auto &dt : dynamic_terminals) {
				if (selected_tab == dt.tab_index) {
					dt.panel->SetCapturing(true);
				}
			}
		}

		// Check if any active terminal tab is capturing input.
		bool any_capturing = false;
		if (selected_tab == 2 && terminal_panel.IsCapturing()) {
			any_capturing = true;
		}
		for (auto &dt : dynamic_terminals) {
			if (selected_tab == dt.tab_index && dt.panel->IsCapturing()) {
				any_capturing = true;
			}
		}

		auto panel = interactive->Render() | borderRounded;
		if (any_capturing)
			panel = panel | color(Color::LightGreen);

		return vbox({ SystemResourcesPanel::Render(),
					  separatorCharacter("*") | bold | color(Color::Orange3),
					  panel,
					  server_content->Render() }) |
			   flex;
	});

	// When a terminal tab is active, intercept keyboard events before
	// the Toggle component consumes them (e.g. arrow keys, Tab, chars).
	auto root = container | CatchEvent([&](Event event) {
					if (selected_tab == 2 && terminal_panel.WantsEvent(event)) {
						return terminal_panel.HandleEvent(event);
					}
					for (auto &dt : dynamic_terminals) {
						if (selected_tab == dt.tab_index &&
							dt.panel->WantsEvent(event)) {
							return dt.panel->HandleEvent(event);
						}
					}
					return false;
				});

	screen.Loop(root);
}
