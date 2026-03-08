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

	auto interactive = Container::Vertical({ tab_toggle, tab_container }) |
					   borderRounded | flex;

	auto container = Renderer(interactive, [&] {
		if (selected_tab == 2 && !terminal_panel.IsSpawned()) {
			terminal_panel.Spawn();
		}
		return vbox({ SystemResourcesPanel::Render(),
					  separatorCharacter("*") | bold | color(Color::Orange3),
					  interactive->Render(),
					  server_content->Render() }) |
			   flex;
	});

	// When the Terminal tab is active, intercept keyboard events before
	// the Toggle component consumes them (e.g. arrow keys, Tab, chars).
	auto root = container | CatchEvent([&](Event event) {
					if (selected_tab == 2 && terminal_panel.WantsEvent(event)) {
						return terminal_panel.HandleEvent(event);
					}
					return false;
				});

	screen.Loop(root);
}
