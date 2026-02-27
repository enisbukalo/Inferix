#include "ram_monitor.h"
#include "system_resources.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element SystemResources::Render() {
	MemoryMonitor::instance().update();
	auto ram_stats = MemoryMonitor::instance().get_stats();

	auto table = Table({
			{"Total", std::to_string(ram_stats.total_bytes)},
			{"Used", std::to_string(ram_stats.used_bytes)},
			{"Available", std::to_string(ram_stats.available_bytes)},
			{"Usage", std::to_string(ram_stats.usage_percentage) + "%"},
			});
	table.SelectColumn(0).Decorate(color(Color::MagentaLight));
	table.SelectColumn(1).Decorate(color(Color::LightGreen));
	table.SelectColumn(0).Border(ftxui::DASHED);
	table.SelectAll().Border(ftxui::DOUBLE);

	return table.Render();
}
