#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>

/**
 * @file terminalPresetsPanel.h
 * @brief Panel for managing user-defined terminal presets.
 *
 * Allows users to create, edit, and delete terminal presets that appear
 * as dynamic tabs in the main application window.
 */
class TerminalPresetsPanel
{
  public:
	/**
	 * @brief Renders the terminal presets panel.
	 *
	 * Displays a table of existing presets with options to remove them,
	 * and a form to add new presets.
	 *
	 * @return FTXUI element representing the panel.
	 */
	static ftxui::Element render();

	/**
	 * @brief Adds a new terminal preset.
	 *
	 * @param name Display name for the tab (e.g., "Opencode", "GitUI")
	 * @param command Command to execute (e.g., "opencode", "gitui")
	 * @return true if preset was added, false if name already exists
	 */
	static bool addPreset(const std::string &name, const std::string &command);

	/**
	 * @brief Removes an existing terminal preset by name.
	 *
	 * @param name Name of the preset to remove
	 * @return true if preset was found and removed, false otherwise
	 */
	static bool removePreset(const std::string &name);

  private:
	// Edit state for adding new presets
	static std::string newPresetName_;
	static std::string newPresetCommand_;
};
