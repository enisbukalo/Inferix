#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @brief Stateless panel that renders detected hardware information.
 *
 * Reads CPU and GPU identifiers from @c SystemInfo and formats them
 * into an FTXUI element suitable for display in the TUI layout.
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
