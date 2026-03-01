#include "ram_monitor.h"
#include "system_resources_panel.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <sstream>

using namespace ftxui;

Element SystemResourcesPanel::Render() {
	auto ram_stats = MemoryMonitor::instance().get_stats();

	std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
	oss_total << std::fixed << std::setprecision(2) << (ram_stats.total_mb / 1024.0);
	oss_used << std::fixed << std::setprecision(2) << (ram_stats.used_mb / 1024.0);
	oss_avail << std::fixed << std::setprecision(2) << (ram_stats.available_mb / 1024.0);
	oss_pct << std::fixed << std::setprecision(2) << ram_stats.usage_percentage;

	auto table = Table({
			{"Total", oss_total.str() + " GB"},
			{"Used", oss_used.str() + " GB"},
			{"Available", oss_avail.str() + " GB"},
			{"Usage", oss_pct.str() + "%"},
			});
	table.SelectColumn(0).Decorate(color(Color::MagentaLight));
	table.SelectColumn(1).Decorate(color(Color::LightGreen));
	table.SelectColumn(0).Border(ftxui::DASHED);

	return table.Render();
}
