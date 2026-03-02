#include "server_info_panel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element ServerInfoPanel::Render() {
	static int frame = 0;
	frame++;

	bool connected = (frame / 4) % 2 == 0;
	Color pulse_color = connected ? (frame % 2 == 0 ? Color::Green : Color::GreenLight) : (frame % 2 == 0 ? Color::Red : Color::RedLight);

	return vbox({text("Server Status") | bold | hcenter,
				 separator(),
				 hbox({
					 text("Server Status") | bold,
					 separatorEmpty(),
					 text("◉") | hcenter | color(pulse_color),
				 })}) |
		   borderRounded;
}
