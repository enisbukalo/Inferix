/**
 * @file system_info_panel.cpp
 * @brief System information panel implementation.
 *
 * Implements a panel that displays detected CPU and GPU hardware
 * information in a table format.
 */

#include "system_info_panel.h"
#include "system_info.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element SystemInfoPanel::Render()
{
	SystemInfo::instance().update();
	auto cpu = SystemInfo::instance().get_cpu();
	auto gpus = SystemInfo::instance().get_gpus();

	auto padding = [](Element e) { return hbox({ e, text(" ") }); };

	std::vector<std::vector<std::string>> data = {
		{ "CPU", cpu.make, cpu.model },
	};

	for (size_t i = 0; i < gpus.size(); i++) {
		data.push_back(
			{ "GPU " + std::to_string(i), gpus[i].make, gpus[i].model });
	}

	Table table({ data });
	table.SelectAll().DecorateCells(padding);
	if (data.size() > 1) {
		table.SelectRows(1, data.size() - 1)
			.DecorateCells(bgcolor(Color::GreenLight));
		table.SelectRows(1, data.size() - 1).DecorateCells(color(Color::Black));
	}
	return hbox({
		vbox({
			text("System Information") | bold | hcenter,
			separator(),
			table.Render(),
		}) | borderRounded,
	});
}
