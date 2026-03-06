/**
 * @file server_info_panel.cpp
 * @brief Server status panel implementation.
 *
 * Implements an animated status indicator that pulses between colors
 * to show server connection state.
 */

#include "server_info_panel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element ServerInfoPanel::Render() {
	static int frame = 0;
	frame++;

	Color pulse_color = frame < 8 ? Color::Red : Color::GreenLight; // 8 frames at 4fps = 2 seconds

	return hbox({
		text("Server Status") | bold,
		separatorEmpty(),
		text("◉") | hcenter | color(pulse_color),
	});
}
