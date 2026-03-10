#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @file modelsPanel.h
 * @brief Panel that displays the list of available AI/ML models for inference.
 *
 * This panel provides a scrollable list view of available model files
 * that can be loaded for inference. Each entry includes:
 * - Selection checkbox: For selecting the model
 * - Model filename: The GGUF or other model file name
 *
 * The panel uses a vbox layout with flex modifier to enable scrolling
 * when the model list exceeds the available vertical space. Each row
 * is styled with a window border for clear visual separation.
 *
 * @note Currently displays placeholder content; future implementations
 *       will populate this with actual model metadata from the filesystem.
 */
class ModelsPanel
{
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing available models.
	 *
	 * Renders a scrollable list of model entries with window border styling.
	 * Currently displays placeholder content ready for future model data
	 * integration.
	 *
	 * @return An @c ftxui::Element containing the models list panel.
	 */
	static ftxui::Element Render();

  private:
};
