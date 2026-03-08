#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @brief Animated panel that displays the current server connection status.
 *
 * Uses a static frame counter incremented on each render call to drive a
 * pulsing color indicator. The status toggles between "CONNECTED" and
 * "DISCONNECTED" approximately every 2 seconds, synchronized with the
 * 500ms redraw cycle provided by @c SystemMonitorRunner.
 */
class ServerInfoPanel
{
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing animated server status.
	 *
	 * Increments an internal frame counter to animate a colored dot and status
	 * label. The dot pulses between two shades of green (connected) or red
	 * (disconnected), toggling state every 4 frames (~2 seconds at 500ms/frame).
	 *
	 * @return An @c ftxui::Element containing the server status card.
	 */
	static ftxui::Element Render();

  private:
};
