#include "ram_monitor.h"
#include "system_resources_panel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element SystemResourcesPanel::Render() {
	MemoryMonitor::instance().update();
	auto ram_stats = MemoryMonitor::instance().get_stats();

	auto table = Table({
			{"Total", std::to_string(ram_stats.total_mb) + " MB"},
			{"Used", std::to_string(ram_stats.used_mb) + " MB"},
			{"Available", std::to_string(ram_stats.available_mb) + " MB"},
			{"Usage", std::to_string(ram_stats.usage_percentage) + "%"},
			});
	table.SelectColumn(0).Decorate(color(Color::MagentaLight));
	table.SelectColumn(1).Decorate(color(Color::LightGreen));
	table.SelectColumn(0).Border(ftxui::DASHED);

	return table.Render();
}
