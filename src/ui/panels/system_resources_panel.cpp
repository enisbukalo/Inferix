#include "system_resources_panel.h"
#include "ram_monitor.h"
#include "gpu_monitor.h"
#include "memory_stats.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <sstream>

using namespace ftxui;

std::vector<std::vector<Element>> SystemResourcesPanel::BuildRamRows(const MemoryStats& ram_stats) {
    std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
    oss_total << std::fixed << std::setprecision(2) << (ram_stats.total_mb / 1024.0);
    oss_used << std::fixed << std::setprecision(2) << (ram_stats.used_mb / 1024.0);
    oss_avail << std::fixed << std::setprecision(2) << (ram_stats.available_mb / 1024.0);
    oss_pct << std::fixed << std::setprecision(2) << ram_stats.usage_percentage;

    Element gauge = gaugeRight(ram_stats.usage_percentage / 100.0f) | color(LinearGradient(0, Color::LightGreen, Color::DarkMagenta));

    std::vector<std::vector<Element>> rows = {
        {text("Total"), text(oss_total.str() + " GB")},
        {text("Used"),  text(oss_used.str() + " GB")},
        {text("Avail"), text(oss_avail.str() + " GB")},
        {text("Usage"), text(oss_pct.str() + "%")},
        {text(""),      gauge},
    };

    return rows;
}

std::vector<std::vector<Element>> SystemResourcesPanel::BuildGpuRows(const std::vector<MemoryStats>& gpu_stats) {
    std::vector<std::vector<Element>> rows(5);
    for (size_t i = 0; i < gpu_stats.size(); ++i) {
        const auto& stats = gpu_stats[i];
        std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
        oss_total << std::fixed << std::setprecision(2) << (stats.total_mb / 1024.0);
        oss_used << std::fixed << std::setprecision(2) << (stats.used_mb / 1024.0);
        oss_avail << std::fixed << std::setprecision(2) << (stats.available_mb / 1024.0);
        oss_pct << std::fixed << std::setprecision(2) << stats.usage_percentage;

        Element gauge = gaugeRight(stats.usage_percentage / 100.0f) | color(LinearGradient(0, Color::LightGreen, Color::DarkMagenta));

        rows[0].push_back(text(oss_total.str() + " GB"));
        rows[1].push_back(text(oss_used.str() + " GB"));
        rows[2].push_back(text(oss_avail.str() + " GB"));
        rows[3].push_back(text(oss_pct.str() + "%"));
        rows[4].push_back(gauge);
    }
    return rows;
}

Element SystemResourcesPanel::Render() {
    auto ram_rows = BuildRamRows(MemoryMonitor::instance().get_stats());
    auto gpu_rows = BuildGpuRows(GpuMonitor::instance().get_stats());

    // Append GPU columns to RAM rows
    for (size_t i = 0; i < ram_rows.size() && i < gpu_rows.size(); ++i) {
        for (auto& el : gpu_rows[i]) {
            ram_rows[i].push_back(std::move(el));
        }
    }

    auto padding = [](Element e) {
        return hbox({e, text(" ")});
    };

    Table table(ram_rows);
    table.SelectAll().DecorateCells(padding);

    return hbox({
        vbox({
            text("System Resources") | bold | hcenter,
            separator(),
            table.Render(),
        }) | border,
    });
}
