#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @file inferenceSettingsPanel.h
 * @brief Runtime inference parameters configuration panel.
 *
 * This panel displays the current inference runtime settings for AI model
 * generation, including:
 * - Temperature: Controls randomness in token selection (0.0-1.0)
 * - Top P: Nucleus sampling threshold (0.0-1.0)
 * - Max Tokens: Maximum tokens to generate
 * - Repeat Penalty: Penalizes repeated tokens
 * - Stop Sequences: Custom termination strings
 * - Logits Processor: Whether logits processing is enabled
 *
 * The panel uses a horizontal key-value layout with gauge indicators
 * for percentage-based settings. All values are currently placeholders
 * and would be populated from application configuration in a full
 * implementation.
 */
class InferenceSettingsPanel
{
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
