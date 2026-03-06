#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @brief Panel that displays the list of available AI/ML models for inference.
 *
 * Provides a scrollable view of loaded models with placeholder structure
 * ready for future model entry data. Uses vbox with flex for scrollable content.
 */
class ModelsPanel {
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing available models.
	 *
	 * Renders a scrollable list of model entries with window border styling.
	 * Currently displays placeholder content ready for future model data integration.
	 *
	 * @return An @c ftxui::Element containing the models list panel.
	 */
	static ftxui::Element Render();

  private:
};
