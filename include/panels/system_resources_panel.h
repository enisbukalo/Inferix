#pragma once

#include "memory_stats.h"
#include "processor_stats.h"
#include <ftxui/dom/elements.hpp>
#include <vector>

class SystemResourcesPanel {
  public:
	static ftxui::Element Render();

  private:
	static std::vector<std::vector<ftxui::Element>> BuildRamRows(const MemoryStats &);
	static std::vector<std::vector<ftxui::Element>> BuildGpuRows(const std::vector<MemoryStats> &);
	static std::vector<ftxui::Element> BuildHeaderRow(const std::vector<MemoryStats> &);
	static ftxui::Element BuildCpuGauge(const ProcessorStats &);
	static ftxui::Element BuildGpuGauges(const std::vector<ProcessorStats> &);
	static std::vector<ftxui::Element> BuildUnitsColumn();
};
