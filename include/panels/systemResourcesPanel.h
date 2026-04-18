#pragma once

#include "memoryStats.h"
#include "modelInfoMonitor.h"
#include "modelInfoPanel.h"
#include "processorStats.h"
#include <ftxui/dom/elements.hpp>
#include <vector>

/**
 * @file systemResourcesPanel.h
 * @brief Stateless panel that renders live resource-usage gauges and a memory
 * table.
 *
 * This panel provides a comprehensive view of system resources, including:
 * - CPU load gauge: Vertical bar showing current CPU utilization
 * - GPU load gauges: One vertical bar per detected GPU
 * - Memory table: Tabular display of RAM and VRAM usage with:
 *   - Total capacity
 *   - Used memory
 *   - Available memory
 *   - Usage percentage
 *   - Color-coded gauge indicator
 * - Total memory column: Aggregated RAM + all GPU VRAM
 *
 * The panel is completely stateless; all data is fetched from the
 * monitor singletons (CpuMonitor, MemoryMonitor, GpuMonitor) on
 * each render call. All helper methods are static.
 *
 * @note This panel is designed for high-frequency updates (typically
 *       2Hz via SystemMonitorRunner) and minimizes allocations
 *       during rendering for performance.
 */
class SystemResourcesPanel
{
  public:
	/**
	 * @brief Builds and returns the complete resource-usage panel.
	 *
	 * Queries @c CpuMonitor, @c MemoryMonitor, and @c GpuMonitor for the latest
	 * snapshots, then composes all sub-elements into the full panel layout.
	 *
	 * @return An @c ftxui::Element containing the fully composed panel.
	 */
	static ftxui::Element render();

  private:
	/**
	 * @brief Builds table row data for system RAM usage.
	 *
	 * @param stats The current @c MemoryStats snapshot for system RAM.
	 * @return A 2-D vector of @c ftxui::Element rows suitable for a table
	 * widget.
	 */
	static std::vector<std::vector<ftxui::Element>>
	buildRamRows(const MemoryStats &stats);

	/**
	 * @brief Builds table row data for each GPU's VRAM usage.
	 *
	 * @param stats A vector of @c MemoryStats snapshots, one per GPU.
	 * @return A 2-D vector of @c ftxui::Element rows suitable for a table
	 * widget.
	 */
	static std::vector<std::vector<ftxui::Element>>
	buildGpuRows(const std::vector<MemoryStats> &stats);

	/**
	 * @brief Builds the column-header row for the memory table.
	 *
	 * Generates labels for RAM and each detected GPU (e.g. "GPU 0", "GPU 1", …).
	 *
	 * @param gpu_stats A vector of GPU @c MemoryStats used to determine the GPU
	 * count.
	 * @return A vector of @c ftxui::Element column-header cells.
	 */
	static std::vector<ftxui::Element>
	buildHeaderRow(const std::vector<MemoryStats> &gpu_stats);

	/**
	 * @brief Builds a vertical bar gauge representing CPU load.
	 *
	 * @param stats The current @c ProcessorStats snapshot for the CPU.
	 * @return An @c ftxui::Element containing the CPU load gauge.
	 */
	static ftxui::Element buildCpuGauge(const ProcessorStats &stats);

	/**
	 * @brief Builds vertical bar gauges for each GPU's compute utilisation.
	 *
	 * @param stats A vector of @c ProcessorStats snapshots, one per GPU.
	 * @return An @c ftxui::Element containing all GPU load gauges side by side.
	 */
	static ftxui::Element
	buildGpuGauges(const std::vector<ProcessorStats> &stats);

	/**
	 * @brief Builds the rightmost units column for the memory table.
	 *
	 * Contains unit labels such as "GB" and "%" to annotate memory table rows.
	 *
	 * @return A vector of @c ftxui::Element unit-label cells.
	 */
	static std::vector<ftxui::Element> buildUnitsColumn();

	/**
	 * @brief Builds the total memory column (RAM + all GPUs aggregated).
	 *
	 * Aggregates system RAM and all GPU memory into a single column showing
	 * combined totals, usage, and available memory.
	 *
	 * @param gpuStats A vector of GPU MemoryStats snapshots.
	 * @param ramStats The current MemoryStats snapshot for system RAM.
	 * @return A 2-D vector of ftxui::Element rows (5 rows: Total, Used, Avail,
	 * Usage, Gauge).
	 */
	static std::vector<ftxui::Element>
	buildTotalMemoryColumn(const std::vector<MemoryStats> &gpuStats,
						   const MemoryStats &ramStats);
};
