#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @file modelPresetsPanel.h
 * @brief Panel for displaying and managing preset configurations for AI models.
 *
 * This panel displays predefined inference presets that combine model
 * selection with optimized runtime parameters:
 * - Fast: Smaller models optimized for speed (e.g., llama-2-7b)
 * - Balanced: Mid-sized models with good quality/speed tradeoff (e.g.,
 * mistral-7b)
 * - Quality: Larger models for maximum quality (e.g., llama-2-13b)
 *
 * Each preset row includes:
 * - Preset name: Human-readable identifier
 * - Model name: The associated model file
 * - Selection indicator: Checkbox for selecting the preset
 *
 * The panel uses a table layout with alternating row colors and a
 * dark header row for visual distinction.
 */
class ModelPresetsPanel
{
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
