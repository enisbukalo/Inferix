/**
 * @file loadSettingsPanel.cpp
 * @brief Load settings panel implementation.
 *
 * Implements a panel that displays model loading configuration settings
 * in a table format with alternating row colors.
 */

#include "loadSettingsPanel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

Element LoadSettingsPanel::render()
{
	// Header row
	std::vector<Element> header = {
		text("Setting") | bold,
		text("Value") | bold,
	};

	// Data rows
	std::vector<std::vector<Element>> rows = {
		{ text("Model Path"), text("/models/") },
		{ text("Format"), text("GGUF") },
		{ text("Cache Size"), text("2048 MB") },
		{ text("Auto-Load"), text("Enabled") },
		{ text("Backend"), text("CPU+GPU") },
	};

	// Insert header at beginning
	rows.insert(rows.begin(), { header });

	Table table(rows);
	auto padding = [](Element e) { return hbox({ e, text(" ") }); };
	table.SelectAll().DecorateCells(padding);

	// Decorate header row
	table.SelectRows(0, 1).DecorateCells(bgcolor(Color::Black));
	table.SelectRows(0, 1).DecorateCells(color(Color::White));

	// Decorate data rows with alternating colors
	auto data_rows = table.SelectRows(1, -1);
	data_rows.DecorateCells(color(Color::MagentaLight));

	return window(text("Load Settings") | bold,
				  vbox({
					  table.Render(),
				  }));
}
