#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @file serverInfoPanel.h
 * @brief Animated panel that displays the current server connection status.
 *
 * This panel provides a visual indicator of the inference server's connection
 * state. The animation uses a frame counter that increments on each render
 * call to create a pulsing effect:
 *
 * - Frames 0-7: Green indicator, "CONNECTED" status
 * - Frames 8-15: Red indicator, "DISCONNECTED" status
 * - Cycle repeats every 16 frames (~2 seconds at 8fps)
 *
 * The status toggle is synchronized with the 500ms redraw cycle from
 * SystemMonitorRunner, resulting in approximately 2-second intervals
 * between state changes.
 *
 * @note This is a placeholder implementation; actual server connection
 *       status would be determined by monitoring the inference server's
 *       health endpoint or socket connection.
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
