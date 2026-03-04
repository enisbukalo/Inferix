#include "gpu_monitor.h"
#include "ram_monitor.h"
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
		return vbox({
			hbox({
				SystemResourcesPanel::Render(),
			}),
		});
	});

	screen.Loop(container);

	return 0;
}
