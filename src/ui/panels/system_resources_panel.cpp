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

	auto padding = [](Element e) {
		return hbox({e, text(" ")});
	};

	std::ostringstream oss_total, oss_used, oss_avail, oss_pct;
	oss_total << std::fixed << std::setprecision(2) << (ram_stats.total_mb / 1024.0);
	oss_used << std::fixed << std::setprecision(2) << (ram_stats.used_mb / 1024.0);
	oss_avail << std::fixed << std::setprecision(2) << (ram_stats.available_mb / 1024.0);
	oss_pct << std::fixed << std::setprecision(2) << ram_stats.usage_percentage;

	Element gauge = gaugeRight(ram_stats.usage_percentage / 100.0f) | color(LinearGradient(0, Color::LightGreen, Color::DarkMagentaB));

	std::vector<std::vector<Element>> data = {
		{text("Total"), text(oss_total.str() + " GB")},
		{text("Used"),  text(oss_used.str() + " GB")},
		{text("Avail"), text(oss_avail.str() + " GB")},
		{text("Usage"), text(oss_pct.str() + "%")},
		{text(""),      gauge},
	};

	Table table(data);
	table.SelectAll().DecorateCells(padding);

	return hbox({
		vbox({
			text("Memory Information") | bold | hcenter,
			separator(),
			table.Render(),
		}) | border,
	});
}
