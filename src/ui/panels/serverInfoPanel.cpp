/**
 * @file serverInfoPanel.cpp
 * @brief Animated presence indicator implementation for server display.
 *
 * This is a placeholder animation that alternates between green and red
 * every 8 frames (~2 seconds at ~4fps) to create a simple pulsing effect.
 * No actual server health monitoring is performed.
 */

#include "serverInfoPanel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element ServerInfoPanel::render()
{
	static int frame = 0;
	frame++;

	Color pulse_color = frame < 8
							? Color::Red
							: Color::GreenLight; // 8 frames at 4fps = 2 seconds

	return hbox({
		text("Server Status") | bold,
		separatorEmpty(),
		text("◉") | hcenter | color(pulse_color),
	});
}
