#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @file loadSettingsPanel.h
 * @brief Configuration panel for loading AI/ML models.
 *
 * This panel displays the current model loading configuration settings:
 * - Model Path: Directory where models are stored
 * - Format: Model file format (e.g., GGUF, ONNX)
 * - Cache Size: Maximum cache size for model loading
 * - Auto-Load: Whether to automatically load models on startup
 * - Backend: Hardware backend for inference (CPU, GPU, or hybrid)
 *
 * The panel uses a table layout with alternating row colors for visual
 * distinction. The header row has a dark background with white text.
 */
class LoadSettingsPanel
{
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
