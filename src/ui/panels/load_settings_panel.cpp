#include "load_settings_panel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>

using namespace ftxui;

Element LoadSettingsPanel::Render() {
	// Header row
	std::vector<Element> header = {
		text("Setting") | bold,
		text("Value") | bold,
	};

	// Data rows
	std::vector<std::vector<Element>> rows = {
		{text("Model Path") | color(Color::CyanLight), text("/models/") | color(Color::CyanLight)},
		{text("Format") | color(Color::MagentaLight), text("GGUF") | color(Color::MagentaLight)},
		{text("Cache Size") | color(Color::CyanLight), text("2048 MB") | color(Color::CyanLight)},
		{text("Auto-Load") | color(Color::MagentaLight), text("Enabled") | color(Color::MagentaLight)},
		{text("Backend") | color(Color::CyanLight), text("CPU+GPU") | color(Color::CyanLight)},
	};

	// Insert header at beginning
	rows.insert(rows.begin(), {header});

	Table table(rows);
	auto padding = [](Element e) { return hbox({e, text(" ")}); };
	table.SelectAll().DecorateCells(padding);

	// Decorate header row
	table.SelectRows(0, 1).DecorateCells(bgcolor(Color::Black));
	table.SelectRows(0, 1).DecorateCells(color(Color::White));

	// Decorate data rows with alternating colors
	auto data_rows = table.SelectRows(1, -1);
	data_rows.DecorateCellsAlternateRow(color(Color::CyanLight), 2, 1);
	data_rows.DecorateCellsAlternateRow(color(Color::MagentaLight), 2, 0);

	return window(text("Load Settings") | bold,
				  vbox({
					  separatorLight(),
					  table.Render(),
					  separatorLight(),
				  }));
}
