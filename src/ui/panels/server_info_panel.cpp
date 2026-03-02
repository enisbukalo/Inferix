#include "server_info_panel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element ServerInfoPanel::Render() {
	static int frame = 0;
	frame++;

	bool connected = (frame / 4) % 2 == 0;
	Color pulse_color = connected ? (frame % 2 == 0 ? Color::Green : Color::GreenLight) : (frame % 2 == 0 ? Color::Red : Color::RedLight);
	Color status_color = connected ? Color::Green : Color::Red;

	return vbox({
			   text("Server Status") | bold | hcenter,
			   separator(),
			   text("  ●  ") | borderRounded | hcenter | color(pulse_color),
			   text(connected ? "CONNECTED" : "DISCONNECTED") | bold | hcenter | color(status_color),
		   }) |
		   borderRounded;
}
