#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @brief Runtime inference parameters configuration panel.
 *
 * Provides a settings interface for inference-time parameters such as
 * temperature, top_p, max_tokens, and other runtime options. Uses
 * key-value layout with visual indicators where appropriate.
 */
class InferenceSettingsPanel {
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing inference settings.
	 *
	 * Renders key-value pairs for temperature, top_p, max_tokens,
	 * and other runtime parameters. Uses gauge indicators for
	 * percentage-based settings.
	 *
	 * @return An @c ftxui::Element containing the inference settings panel.
	 */
	static ftxui::Element Render();

  private:
};
