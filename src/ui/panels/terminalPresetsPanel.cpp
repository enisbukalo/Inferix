/**
 * @file terminalPresetsPanel.cpp
 * @brief Terminal presets panel implementation.
 *
 * Implements a panel that displays user-defined terminal presets
 * in a table format with options to add and remove presets.
 */

#include "terminalPresetsPanel.h"
#include "configManager.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

// Static member initialization
std::string TerminalPresetsPanel::newPresetName_;
std::string TerminalPresetsPanel::newPresetCommand_;

bool TerminalPresetsPanel::addPreset(const std::string &name,
									 const std::string &command)
{
	if (name.empty() || command.empty())
		return false;

	Config::TerminalPreset preset;
	preset.name = name;
	preset.initialCommand = command;

	return ConfigManager::instance().addTerminalPreset(std::move(preset));
}

bool TerminalPresetsPanel::removePreset(const std::string &name)
{
	return ConfigManager::instance().removeTerminalPreset(name);
}

Element TerminalPresetsPanel::render()
{
	// Get all presets from config
	const auto &presets = ConfigManager::instance().getTerminalPresets();

	// Header row
	std::vector<Element> header = {
		text("Name") | bold,
		text("Command") | bold,
		text("Action") | bold,
	};

	// Data rows
	std::vector<std::vector<Element>> rows;
	for (size_t i = 0; i < presets.size(); ++i) {
		const auto &preset = presets[i];
		bool isEven = (i % 2) == 0;

		rows.push_back(
			{ text(preset.name) |
				  (isEven ? color(Color::CyanLight) : color(Color::Cyan)),
			  text(preset.initialCommand) |
				  (isEven ? color(Color::CyanLight) : color(Color::Cyan)),
			  text("[Remove]") | bold |
				  (isEven ? color(Color::RedLight) : color(Color::Red)) });
	}

	// Add form row
	rows.push_back({ text(newPresetName_) | underlined,
					 text(newPresetCommand_) | underlined,
					 text("[Add]") | bold | color(Color::Green) });

	// Insert header at beginning
	rows.insert(rows.begin(), { header });

	Table table(rows);
	auto padding = [](Element e) { return hbox({ e, text(" ") }); };
	table.SelectAll().DecorateCells(padding);

	// Decorate header row
	table.SelectRows(0, 1).DecorateCells(bgcolor(Color::Black));
	table.SelectRows(0, 1).DecorateCells(color(Color::White));

	return window(text("Terminal Presets") | bold,
				  vbox({
					  table.Render(),
				  }));
}
