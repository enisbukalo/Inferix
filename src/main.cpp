#include "system_info_panel.h"
#include "system_resources_panel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

int main(){
	auto screen = ScreenInteractive::Fullscreen();

	auto container = Renderer([] {
		return vbox({
			hbox({
				SystemInfoPanel::Render(),
				filler(),
				text("Server Status") | hcenter | vcenter,
				filler(),
				SystemResourcesPanel::Render(),
			}),
		});
	});

	screen.Loop(container);

	return 0;
}
