#pragma once

#include <ftxui/dom/elements.hpp>
#include <vector>
#include "memory_stats.h"

class SystemResourcesPanel {
public:
    static ftxui::Element Render();
private:
    static std::vector<std::vector<ftxui::Element>> BuildRamRows(const MemoryStats&);
    static std::vector<std::vector<ftxui::Element>> BuildGpuRows(const std::vector<MemoryStats>&);
};
