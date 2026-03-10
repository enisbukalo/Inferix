/**
 * @file system_resources_panel.cpp
 * @brief System resources panel implementation.
 *
 * Implements the main system monitoring panel that displays CPU/GPU load
 * gauges and a comprehensive memory usage table for RAM and all GPUs.
 */

#include "system_resources_panel.h"
#include "cpu_monitor.h"
#include "gpu_monitor.h"
#include "memory_stats.h"
#include "processor_stats.h"
#include "ram_monitor.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <sstream>

using namespace ftxui;

namespace {
const ftxui::LinearGradient kGaugeGradient = ftxui::LinearGradient()
												 .Angle(90)
												 .Stop(Color::Red1)
												 .Stop(Color::Yellow1)
												 .Stop(Color::Green1);
const ftxui::LinearGradient kMemoryGradient = ftxui::LinearGradient()
												  .Angle(0)
												  .Stop(Color::Green1)
												  .Stop(Color::Yellow1)
												  .Stop(Color::Red1);
} // namespace

std::vector<std::vector<Element>>
SystemResourcesPanel::BuildRamRows(const MemoryStats &ramStats)
{
	std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
	oss_total << std::fixed << std::setprecision(2)
			  << (ramStats.totalMb / 1024.0);
	oss_used << std::fixed << std::setprecision(2) << (ramStats.usedMb / 1024.0);
	oss_avail << std::fixed << std::setprecision(2)
			  << (ramStats.availableMb / 1024.0);
	oss_pct << std::fixed << std::setprecision(2) << ramStats.usagePercentage;

	Element gauge =
		gaugeRight(ramStats.usagePercentage / 100.0f) | color(kMemoryGradient);

	std::vector<std::vector<Element>> rows = {
		{ text("Total") | bold, text(oss_total.str()) },
		{ text("Used") | bold, text(oss_used.str()) },
		{ text("Avail") | bold, text(oss_avail.str()) },
		{ text("Usage") | bold, text(oss_pct.str()) },
		{ text(""), gauge },
	};

	return rows;
}

std::vector<std::vector<Element>>
SystemResourcesPanel::BuildGpuRows(const std::vector<MemoryStats> &gpuStats)
{
	std::vector<std::vector<Element>> rows(5);
	for (size_t i = 0; i < gpuStats.size(); ++i) {
		const auto &stats = gpuStats[i];
		std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
		oss_total << std::fixed << std::setprecision(2)
				  << (stats.totalMb / 1024.0);
		oss_used << std::fixed << std::setprecision(2)
				 << (stats.usedMb / 1024.0);
		oss_avail << std::fixed << std::setprecision(2)
				  << (stats.availableMb / 1024.0);
		oss_pct << std::fixed << std::setprecision(2) << stats.usagePercentage;

		Element gauge =
			gaugeRight(stats.usagePercentage / 100.0f) | color(kMemoryGradient);

		rows[0].push_back(text(oss_total.str()));
		rows[1].push_back(text(oss_used.str()));
		rows[2].push_back(text(oss_avail.str()));
		rows[3].push_back(text(oss_pct.str()));
		rows[4].push_back(gauge);
	}
	return rows;
}

Element SystemResourcesPanel::BuildCpuGauge(const ProcessorStats &processorStats)
{
	auto toReturn =
		hbox({
			vtext("CPU") | vcenter | hcenter | bgcolor(Color::NavyBlue),
			separatorLight(),
			gaugeUp(processorStats.usagePercentage / 100.0) |
				ftxui::color(kGaugeGradient),
		}) |
		borderRounded;
	return toReturn;
}

ftxui::Element SystemResourcesPanel::BuildGpuGauges(
	const std::vector<ProcessorStats> &gpuLoadStats)
{
	if (gpuLoadStats.empty())
		return ftxui::text("No GPU");

	Elements gauges;
	for (size_t i = 0; i < gpuLoadStats.size(); ++i) {
		gauges.push_back(hbox({
							 vtext("GPU" + std::to_string(i)) | vcenter |
								 hcenter | bgcolor(Color::NavyBlue),
							 separatorLight(),
							 gaugeUp(gpuLoadStats[i].usagePercentage / 100.0) |
								 ftxui::color(kGaugeGradient),
						 }) |
						 borderRounded);
	}
	return hbox(std::move(gauges));
}

std::vector<ftxui::Element> SystemResourcesPanel::BuildTotalMemoryColumn(
	const std::vector<MemoryStats> &gpuStats,
	const MemoryStats &ramStats)
{
	// Calculate totals
	uint64_t totalMb = ramStats.totalMb;
	uint64_t usedMb = ramStats.usedMb;
	uint64_t availableMb = ramStats.availableMb;

	for (const auto &gpu : gpuStats) {
		totalMb += gpu.totalMb;
		usedMb += gpu.usedMb;
		availableMb += gpu.availableMb;
	}

	// Calculate usage percentage
	double usagePercentage = 0.0;
	if (totalMb > 0) {
		usagePercentage =
			(static_cast<double>(usedMb) / static_cast<double>(totalMb)) * 100.0;
	}

	// Format values
	std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
	oss_total << std::fixed << std::setprecision(2) << (totalMb / 1024.0);
	oss_used << std::fixed << std::setprecision(2) << (usedMb / 1024.0);
	oss_avail << std::fixed << std::setprecision(2) << (availableMb / 1024.0);
	oss_pct << std::fixed << std::setprecision(2) << usagePercentage;

	// Build gauge
	Element gauge =
		gaugeRight(usagePercentage / 100.0f) | color(kMemoryGradient);

	// Return column as transposed (5 rows, 1 column each)
	return { text(oss_total.str()),
			 text(oss_used.str()),
			 text(oss_avail.str()),
			 text(oss_pct.str()),
			 gauge };
}

std::vector<Element>
SystemResourcesPanel::BuildHeaderRow(const std::vector<MemoryStats> &gpuStats)
{
	std::vector<Element> header;
	header.push_back(text("")); // Empty for label column
	header.push_back(text("RAM") | bold);
	for (const auto &gpu : gpuStats) {
		header.push_back(text("GPU " + std::to_string(gpu.id)) | bold);
	}
	header.push_back(text("TOTAL") | bold);
	return header;
}

std::vector<Element> SystemResourcesPanel::BuildUnitsColumn()
{
	return {
		text(""),		   // Empty for label
		text("GB") | bold, // Total
		text("GB") | bold, // Used
		text("GB") | bold, // Avail
		text("%") | bold,  // Usage
		text(""),		   // gauge (empty)
	};
}

Element SystemResourcesPanel::Render()
{
	auto ramStats = MemoryMonitor::instance().get_stats();
	auto gpuStats = GpuMonitor::instance().get_stats();
	auto processorStats = CpuMonitor::instance().get_stats();
	auto gpuLoadStats = GpuMonitor::instance().get_load_stats();

	auto ram_rows = BuildRamRows(ramStats);
	auto gpu_rows = BuildGpuRows(gpuStats);
	auto header_row = BuildHeaderRow(gpuStats);
	auto units_column = BuildUnitsColumn();
	auto cpu_load_gauge = BuildCpuGauge(processorStats);
	auto gpu_load_gauges = BuildGpuGauges(gpuLoadStats);
	auto total_memory_column = BuildTotalMemoryColumn(gpuStats, ramStats);

	// Insert header row at the beginning
	ram_rows.insert(ram_rows.begin(), header_row);

	// Append GPU columns to RAM rows
	for (size_t i = 1; i < ram_rows.size() && i < gpu_rows.size() + 1; ++i) {
		for (auto &el : gpu_rows[i - 1]) {
			ram_rows[i].push_back(std::move(el));
		}
	}

	// Add total memory column after GPU columns (skip header row at index 0)
	for (size_t i = 1; i < ram_rows.size(); ++i) {
		ram_rows[i].push_back(total_memory_column[i - 1]);
	}

	// Add units column after RAM, GPU, and total columns
	for (size_t i = 0; i < ram_rows.size(); ++i) {
		ram_rows[i].push_back(units_column[i]);
	}

	// This is here in case we want to add padding
	auto padding = [](Element e) { return hbox({ text(" "), e, text(" ") }); };

	Table table(ram_rows);
	table.SelectAll().DecorateCells(padding);
	auto all_cells = table.SelectAll();
	all_cells.SeparatorVertical(DASHED);

	// Apply alternating CyanLight and MagentaLight to data rows (excluding
	// header)
	auto data_rows =
		table.SelectRows(1, -1); // Select rows 1 to end (skip header)
	data_rows.DecorateCellsAlternateRow(color(Color::CyanLight), 2, 1);
	data_rows.DecorateCellsAlternateRow(color(Color::MagentaLight), 2, 0);

	return window(text("System Resources") | bold,
				  hbox({
					  vbox({
						  text("Memory") | bold | hcenter,
						  separatorLight(),
						  table.Render(),
					  }),
					  separatorHeavy(),
					  vbox({
						  text("Load") | hcenter | bold,
						  separatorLight(),
						  hbox({
							  cpu_load_gauge,
							  separatorLight(),
							  gpu_load_gauges,
						  }),
					  }),
					  separatorLight(),
				  }));
}
