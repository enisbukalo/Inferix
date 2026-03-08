#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @file system_info_panel.h
 * @brief Stateless panel that renders detected hardware information.
 *
 * This panel displays the detected CPU and GPU hardware information
 * obtained from the SystemInfo singleton. The information is presented
 * in a table format with alternating row colors for visual distinction.
 *
 * The panel queries SystemInfo::instance() on each render call to get
 * the latest hardware information, ensuring the display is always
 * up-to-date with the current system state.
 *
 * @note This panel is stateless; all data is fetched from SystemInfo
 *       on each render call. No internal state is maintained.
 */
class SystemInfoPanel
{
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing hardware identifiers.
	 *
	 * Queries the @c SystemInfo singleton for the current CPU and GPU
	 * descriptors and composes them into a formatted display element.
	 *
	 * @return An @c ftxui::Element containing the hardware info panel.
	 */
	static ftxui::Element Render();

  private:
};
