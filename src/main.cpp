#include "gpu_monitor.h"
#include "ram_monitor.h"
#include "server_info_panel.h"
#include "system_monitor_runner.h"
#include "system_resources_panel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

int main() {
	auto screen = ScreenInteractive::Fullscreen();

	MemoryMonitor::instance().update();
	GpuMonitor::instance().update();

	SystemMonitorRunner runner(screen);

	auto container = Renderer([] {
		return vbox({hbox({SystemResourcesPanel::Render()}),
					 hbox({vbox({window(text("Models") | bold, vbox({}) | flex), window(text("Model Presets") | bold, vbox({}) | flex)}) | flex,
						   window(text("Model Chat"), vbox({}) | flex),
						   vbox({window(text("Load Settings") | bold, vbox({}) | flex), window(text("Inference Settings") | bold, vbox({}) | flex)}) | flex}) |
						 flex});
	});

	screen.Loop(container);

	return 0;
}
