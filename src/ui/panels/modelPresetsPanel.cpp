/**
 * @file modelPresetsPanel.cpp
 * @brief Model presets panel implementation.
 *
 * Implements a panel that displays preset configurations for AI models
 * in a table format with alternating row colors.
 */

#include "modelPresetsPanel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

Element ModelPresetsPanel::Render()
{
	// Header row
	std::vector<Element> header = {
		text("Preset Name") | bold,
		text("Model") | bold,
		text("Select") | bold,
	};

	// Data rows
	std::vector<std::vector<Element>> rows = {
		{ text("Fast"), text("llama-2-7b"), text("[ ]") | color(Color::Black) },
		{ text("Balanced"),
		  text("mistral-7b"),
		  text("[ ]") | color(Color::Black) },
		{ text("Quality"),
		  text("llama-2-13b"),
		  text("[ ]") | color(Color::Black) },
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
	data_rows.DecorateCells(color(Color::CyanLight));

	return window(text("Model Presets") | bold,
				  vbox({
					  table.Render(),
				  }));
}
