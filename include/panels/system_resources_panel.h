#pragma once

#include "memory_stats.h"
#include "processor_stats.h"
#include <ftxui/dom/elements.hpp>
#include <vector>

/**
 * @brief Stateless panel that renders live resource-usage gauges and a memory table.
 *
 * Composes CPU load gauges, GPU load gauges, and a tabular memory breakdown
 * (RAM + per-GPU VRAM) into a single FTXUI element. All methods are static;
 * the class holds no instance state.
 */
class SystemResourcesPanel {
  public:
	/**
	 * @brief Builds and returns the complete resource-usage panel.
	 *
	 * Queries @c CpuMonitor, @c MemoryMonitor, and @c GpuMonitor for the latest
	 * snapshots, then composes all sub-elements into the full panel layout.
	 *
	 * @return An @c ftxui::Element containing the fully composed panel.
	 */
	static ftxui::Element Render();

  private:
	/**
	 * @brief Builds table row data for system RAM usage.
	 *
	 * @param stats The current @c MemoryStats snapshot for system RAM.
	 * @return A 2-D vector of @c ftxui::Element rows suitable for a table widget.
	 */
	static std::vector<std::vector<ftxui::Element>> BuildRamRows(const MemoryStats &stats);

	/**
	 * @brief Builds table row data for each GPU's VRAM usage.
	 *
	 * @param stats A vector of @c MemoryStats snapshots, one per GPU.
	 * @return A 2-D vector of @c ftxui::Element rows suitable for a table widget.
	 */
	static std::vector<std::vector<ftxui::Element>> BuildGpuRows(const std::vector<MemoryStats> &stats);

	/**
	 * @brief Builds the column-header row for the memory table.
	 *
	 * Generates labels for RAM and each detected GPU (e.g. "GPU 0", "GPU 1", …).
	 *
	 * @param gpu_stats A vector of GPU @c MemoryStats used to determine the GPU count.
	 * @return A vector of @c ftxui::Element column-header cells.
	 */
	static std::vector<ftxui::Element> BuildHeaderRow(const std::vector<MemoryStats> &gpu_stats);

	/**
	 * @brief Builds a vertical bar gauge representing CPU load.
	 *
	 * @param stats The current @c ProcessorStats snapshot for the CPU.
	 * @return An @c ftxui::Element containing the CPU load gauge.
	 */
	static ftxui::Element BuildCpuGauge(const ProcessorStats &stats);

	/**
	 * @brief Builds vertical bar gauges for each GPU's compute utilisation.
	 *
	 * @param stats A vector of @c ProcessorStats snapshots, one per GPU.
	 * @return An @c ftxui::Element containing all GPU load gauges side by side.
	 */
	static ftxui::Element BuildGpuGauges(const std::vector<ProcessorStats> &stats);

	/**
	 * @brief Builds the rightmost units column for the memory table.
	 *
	 * Contains unit labels such as "GB" and "%" to annotate memory table rows.
	 *
	 * @return A vector of @c ftxui::Element unit-label cells.
	 */
	static std::vector<ftxui::Element> BuildUnitsColumn();
};
