#include "system_resources_panel.h"
#include "gpu_monitor.h"
#include "memory_stats.h"
#include "ram_monitor.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <sstream>

using namespace ftxui;

std::vector<std::vector<Element>> SystemResourcesPanel::BuildRamRows(const MemoryStats &ram_stats) {
	std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
	oss_total << std::fixed << std::setprecision(2) << (ram_stats.total_mb / 1024.0);
	oss_used << std::fixed << std::setprecision(2) << (ram_stats.used_mb / 1024.0);
	oss_avail << std::fixed << std::setprecision(2) << (ram_stats.available_mb / 1024.0);
	oss_pct << std::fixed << std::setprecision(2) << ram_stats.usage_percentage;

	Element gauge = gaugeRight(ram_stats.usage_percentage / 100.0f) | color(LinearGradient(0, Color::LightGreen, Color::DarkMagenta));

	std::vector<std::vector<Element>> rows = {
		{text("Total") | bold, text(oss_total.str())},
		{text("Used") | bold, text(oss_used.str())},
		{text("Avail") | bold, text(oss_avail.str())},
		{text("Usage") | bold, text(oss_pct.str())},
		{text(""), gauge},
	};

	return rows;
}

std::vector<std::vector<Element>> SystemResourcesPanel::BuildGpuRows(const std::vector<MemoryStats> &gpu_stats) {
	std::vector<std::vector<Element>> rows(5);
	for (size_t i = 0; i < gpu_stats.size(); ++i) {
		const auto &stats = gpu_stats[i];
		std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
		oss_total << std::fixed << std::setprecision(2) << (stats.total_mb / 1024.0);
		oss_used << std::fixed << std::setprecision(2) << (stats.used_mb / 1024.0);
		oss_avail << std::fixed << std::setprecision(2) << (stats.available_mb / 1024.0);
		oss_pct << std::fixed << std::setprecision(2) << stats.usage_percentage;

		Element gauge = gaugeRight(stats.usage_percentage / 100.0f) | color(LinearGradient(0, Color::LightGreen, Color::DarkMagenta));

		rows[0].push_back(text(oss_total.str()));
		rows[1].push_back(text(oss_used.str()));
		rows[2].push_back(text(oss_avail.str()));
		rows[3].push_back(text(oss_pct.str()));
		rows[4].push_back(gauge);
	}
	return rows;
}

std::vector<Element> SystemResourcesPanel::BuildHeaderRow(const std::vector<MemoryStats> &gpu_stats) {
	std::vector<Element> header;
	header.push_back(text("")); // Empty for label column
	header.push_back(text("RAM") | bold);
	for (const auto &gpu : gpu_stats) {
		header.push_back(text("GPU " + std::to_string(gpu.id)) | bold);
	}
	return header;
}

std::vector<Element> SystemResourcesPanel::BuildUnitsColumn() {
	return {
		text(""),		   // Empty for label
		text("GB") | bold, // Total
		text("GB") | bold, // Used
		text("GB") | bold, // Avail
		text("%") | bold,  // Usage
		text(""),		   // gauge (empty)
	};
}

Element SystemResourcesPanel::Render() {
	auto ramStats = MemoryMonitor::instance().get_stats();
	auto gpuStats = GpuMonitor::instance().get_stats();

	auto ram_rows = BuildRamRows(ramStats);
	auto gpu_rows = BuildGpuRows(gpuStats);
	auto header_row = BuildHeaderRow(gpuStats);
	auto units_column = BuildUnitsColumn();

	// Insert header row at the beginning
	ram_rows.insert(ram_rows.begin(), header_row);

	// Append GPU columns to RAM rows
	for (size_t i = 1; i < ram_rows.size() && i < gpu_rows.size() + 1; ++i) {
		for (auto &el : gpu_rows[i - 1]) {
			ram_rows[i].push_back(std::move(el));
		}
	}

	// Add units column after RAM and GPU columns
	for (size_t i = 0; i < ram_rows.size(); ++i) {
		ram_rows[i].push_back(units_column[i]);
	}

	auto padding = [](Element e) {
		return hbox({e, text(" ")});
	};

	Table table(ram_rows);
	table.SelectAll().DecorateCells(padding);
	auto all_cells = table.SelectAll();
	all_cells.SeparatorVertical(DASHED);

	// Apply alternating CyanLight and MagentaLight to data rows (excluding header)
	auto data_rows = table.SelectRows(1, -1); // Select rows 1 to end (skip header)
	data_rows.DecorateCellsAlternateRow(color(Color::CyanLight), 2, 1);
	data_rows.DecorateCellsAlternateRow(color(Color::MagentaLight), 2, 0);

	return hbox({
		vbox({
			text("System Resources") | bold | hcenter,
			separator(),
			table.Render(),
		}) | borderRounded,
	});
}
