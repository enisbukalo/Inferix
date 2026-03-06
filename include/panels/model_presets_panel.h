#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @brief Panel for displaying and managing preset configurations for AI models.
 *
 * Presents preset configurations in a table-like layout with preset name,
 * associated model, and quick-select capability structure. Follows the
 * ServerInfoPanel pattern with table formatting.
 */
class ModelPresetsPanel {
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing model presets.
	 *
	 * Renders a formatted list of preset configurations with columns for
	 * preset name, associated model, and selection indicator.
	 *
	 * @return An @c ftxui::Element containing the model presets panel.
	 */
	static ftxui::Element Render();

  private:
};
