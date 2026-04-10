/**
 * @file systemResourcesPanel.cpp
 * @brief System resources panel implementation.
 *
 * Implements the main system monitoring panel that displays CPU/GPU load
 * gauges and a comprehensive memory usage table for RAM and all GPUs.
 */

#include "systemResourcesPanel.h"
#include "cpuMonitor.h"
#include "gpuMonitor.h"
#include "memoryStats.h"
#include "processorStats.h"
#include "ramMonitor.h"

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
SystemResourcesPanel::buildRamRows(const MemoryStats &ramStats)
{
	std::ostringstream ossTotal, ossUsed, ossAvail, ossPct;
	ossTotal << std::fixed << std::setprecision(2)
			 << (ramStats.totalMb / 1024.0);
	ossUsed << std::fixed << std::setprecision(2) << (ramStats.usedMb / 1024.0);
	ossAvail << std::fixed << std::setprecision(2)
			 << (ramStats.availableMb / 1024.0);
	ossPct << std::fixed << std::setprecision(2) << ramStats.usagePercentage;

	Element gauge =
		gaugeRight(ramStats.usagePercentage / 100.0f) | color(kMemoryGradient);

	std::vector<std::vector<Element>> rows = {
		{ text("Total") | bold, text(ossTotal.str()) },
		{ text("Used") | bold, text(ossUsed.str()) },
		{ text("Avail") | bold, text(ossAvail.str()) },
		{ text("Usage") | bold, text(ossPct.str()) },
		{ text(""), gauge },
	};

	return rows;
}

std::vector<std::vector<Element>>
SystemResourcesPanel::buildGpuRows(const std::vector<MemoryStats> &gpuStats)
{
	std::vector<std::vector<Element>> rows(5);
	for (size_t i = 0; i < gpuStats.size(); ++i) {
		const auto &stats = gpuStats[i];
		std::ostringstream ossTotal, ossUsed, ossAvail, ossPct;
		ossTotal << std::fixed << std::setprecision(2)
				 << (stats.totalMb / 1024.0);
		ossUsed << std::fixed << std::setprecision(2) << (stats.usedMb / 1024.0);
		ossAvail << std::fixed << std::setprecision(2)
				 << (stats.availableMb / 1024.0);
		ossPct << std::fixed << std::setprecision(2) << stats.usagePercentage;

		Element gauge =
			gaugeRight(stats.usagePercentage / 100.0f) | color(kMemoryGradient);

		rows[0].push_back(text(ossTotal.str()));
		rows[1].push_back(text(ossUsed.str()));
		rows[2].push_back(text(ossAvail.str()));
		rows[3].push_back(text(ossPct.str()));
		rows[4].push_back(gauge);
	}
	return rows;
}

Element SystemResourcesPanel::buildCpuGauge(const ProcessorStats &processorStats)
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

ftxui::Element SystemResourcesPanel::buildGpuGauges(
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

std::vector<ftxui::Element> SystemResourcesPanel::buildTotalMemoryColumn(
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
	std::ostringstream ossTotal, ossUsed, ossAvail, ossPct;
	ossTotal << std::fixed << std::setprecision(2) << (totalMb / 1024.0);
	ossUsed << std::fixed << std::setprecision(2) << (usedMb / 1024.0);
	ossAvail << std::fixed << std::setprecision(2) << (availableMb / 1024.0);
	ossPct << std::fixed << std::setprecision(2) << usagePercentage;

	// Build gauge
	Element gauge =
		gaugeRight(usagePercentage / 100.0f) | color(kMemoryGradient);

	// Return column as transposed (5 rows, 1 column each)
	return { text(ossTotal.str()),
			 text(ossUsed.str()),
			 text(ossAvail.str()),
			 text(ossPct.str()),
			 gauge };
}

std::vector<Element>
SystemResourcesPanel::buildHeaderRow(const std::vector<MemoryStats> &gpuStats)
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

std::vector<Element> SystemResourcesPanel::buildUnitsColumn()
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

Element SystemResourcesPanel::render()
{
	auto ramStats = MemoryMonitor::instance().getStats();
	auto gpuStats = GpuMonitor::instance().getStats();
	auto processorStats = CpuMonitor::instance().getStats();
	auto gpuLoadStats = GpuMonitor::instance().getLoadStats();

	auto ramRows = buildRamRows(ramStats);
	auto gpuRows = buildGpuRows(gpuStats);
	auto headerRow = buildHeaderRow(gpuStats);
	auto unitsColumn = buildUnitsColumn();
	auto cpuLoadGauge = buildCpuGauge(processorStats);
	auto gpuLoadGauges = buildGpuGauges(gpuLoadStats);
	auto totalMemoryColumn = buildTotalMemoryColumn(gpuStats, ramStats);

	// Insert header row at the beginning
	ramRows.insert(ramRows.begin(), headerRow);

	// Append GPU columns to RAM rows
	for (size_t i = 1; i < ramRows.size() && i < gpuRows.size() + 1; ++i) {
		for (auto &el : gpuRows[i - 1]) {
			ramRows[i].push_back(std::move(el));
		}
	}

	// Add total memory column after GPU columns (skip header row at index 0)
	for (size_t i = 1; i < ramRows.size(); ++i) {
		ramRows[i].push_back(totalMemoryColumn[i - 1]);
	}

	// Add units column after RAM, GPU, and total columns
	for (size_t i = 0; i < ramRows.size(); ++i) {
		ramRows[i].push_back(unitsColumn[i]);
	}

	// This is here in case we want to add padding
	auto padding = [](Element e) { return hbox({ text(" "), e, text(" ") }); };

	Table table(ramRows);
	table.SelectAll().DecorateCells(padding);
	auto allCells = table.SelectAll();
	allCells.SeparatorVertical(DASHED);

	// Apply alternating CyanLight and MagentaLight to data rows (excluding
	// header)
	auto dataRows =
		table.SelectRows(1, -1); // Select rows 1 to end (skip header)
	dataRows.DecorateCellsAlternateRow(color(Color::CyanLight), 2, 1);
	dataRows.DecorateCellsAlternateRow(color(Color::MagentaLight), 2, 0);

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
							  cpuLoadGauge,
							  separatorLight(),
							  gpuLoadGauges,
						  }),
					  }),
					  separatorLight(),
					  filler(),
					  vbox({
						  text("Model Info") | bold | hcenter,
						  separatorLight(),
						  ModelInfoPanel::render(),
					  }),
				  }));
}
