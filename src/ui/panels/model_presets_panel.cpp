/**
 * @file model_presets_panel.cpp
 * @brief Model presets panel implementation.
 *
 * Implements a panel that displays preset configurations for AI models
 * in a table format with alternating row colors.
 */

#include "model_presets_panel.h"

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
		{ text("Fast") | color(Color::CyanLight),
		  text("llama-2-7b") | color(Color::CyanLight),
		  text("[ ]") | color(Color::Black) },
		{ text("Balanced") | color(Color::MagentaLight),
		  text("mistral-7b") | color(Color::MagentaLight),
		  text("[ ]") | color(Color::Black) },
		{ text("Quality") | color(Color::CyanLight),
		  text("llama-2-13b") | color(Color::CyanLight),
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
	data_rows.DecorateCellsAlternateRow(color(Color::CyanLight), 2, 1);
	data_rows.DecorateCellsAlternateRow(color(Color::MagentaLight), 2, 0);

	return window(text("Model Presets") | bold,
				  vbox({
					  separatorLight(),
					  table.Render(),
					  separatorLight(),
				  }));
}
