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

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <string>
#include <vector>

using namespace ftxui;

void App::Run()
{
	auto screen = ScreenInteractive::Fullscreen();
	SystemMonitorRunner runner(screen);

	std::vector<std::string> tab_values{ "Settings", "OpenCode" };
	int selected_tab = 0;
	auto tab_toggle = Toggle(&tab_values, &selected_tab);

	auto settings_content = Renderer([] {
		return window(text(""),
					  hbox({ separatorEmpty(),
							 vbox({ ModelsPanel::Render(),
									ModelPresetsPanel::Render() }) |
								 flex,
							 separatorEmpty(),
							 vbox({ LoadSettingsPanel::Render(),
									InferenceSettingsPanel::Render() }) |
								 flex,
							 separatorEmpty() })) |
			   flex;
	});

	auto server_content = Renderer([] {
		return hbox({ text("Some really long status about the server probably "
						   "here."),
					  filler(),
					  ServerInfoPanel::Render() });
	});

	auto opencode_content =
		Renderer([] { return window(text("") | bold, text("")) | flex; });

	auto tab_container =
		Container::Tab({ settings_content, opencode_content }, &selected_tab);

	auto interactive = Container::Vertical({ tab_toggle, tab_container }) |
					   borderRounded | flex;

	auto container = Renderer(interactive, [&] {
		return vbox({ SystemResourcesPanel::Render(),
					  separatorHeavy(),
					  interactive->Render(),
					  server_content->Render() }) |
			   flex;
	});
	screen.Loop(container);
}
