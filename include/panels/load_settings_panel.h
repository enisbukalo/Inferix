#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @brief Configuration panel for loading AI/ML models.
 *
 * Provides a settings interface with model paths, format selectors,
 * and cache configuration options. Follows the SystemInfoPanel pattern
 * using table layout for keyed settings display.
 */
class LoadSettingsPanel {
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing load settings.
	 *
	 * Renders key-value pairs for model path configuration, format
	 * selectors, and cache settings in a table layout.
	 *
	 * @return An @c ftxui::Element containing the load settings panel.
	 */
	static ftxui::Element Render();

  private:
};
