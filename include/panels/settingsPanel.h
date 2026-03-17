#pragma once

#include <ftxui/dom/elements.hpp>

/**
 * @file settingsPanel.h
 * @brief Configuration panel displaying all application settings.
 *
 * This panel displays the current application configuration loaded
 * from ConfigManager. Settings are organized into categories:
 * - Server Settings
 * - Load Settings
 * - Inference Settings
 * - UI Settings
 * - Terminal Settings
 *
 * Uses hbox/vbox layout (not tables) to allow future replacement
 * of text values with interactive controls.
 */
class SettingsPanel
{
  public:
	/**
	 * @brief Builds and returns an FTXUI element showing all configuration.
	 *
	 * Renders a scrollable panel with categorized settings displayed
	 * using hbox/vbox layouts (not tables) to allow future replacement
	 * of text values with interactive controls (sliders, inputs, etc.).
	 *
	 * Categories displayed:
	 * - Server Settings (host, port, API key, SSL, etc.)
	 * - Load Settings (model path, GPU layers, context size, etc.)
	 * - Inference Settings (temperature, top_p, penalties, etc.)
	 * - UI Settings (theme, default tab, refresh rate, etc.)
	 * - Terminal Settings (shell, command, working directory, etc.)
	 *
	 * @return An @c ftxui::Element containing the settings panel.
	 */
	static ftxui::Element render();

  private:
};
