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

using namespace ftxui;

void App::Run() {
	auto screen = ScreenInteractive::Fullscreen();
	SystemMonitorRunner runner(screen);

	auto container = Renderer([] {
		return vbox({hbox({SystemResourcesPanel::Render()}),
					 hbox({vbox({ModelsPanel::Render(), ModelPresetsPanel::Render()}) | flex,
						   vbox({LoadSettingsPanel::Render(), InferenceSettingsPanel::Render()}) | flex}) |
						 flex});
	});

	screen.Loop(container);
}
