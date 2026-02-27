#include <memory>

#include "httplib.h"
#include "json.hpp"
#include "ram_monitor.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <string>

using namespace ftxui;

int main(){
	MemoryMonitor::instance().update();
	auto ram_stats = MemoryMonitor::instance().get_stats();
	auto screen = ScreenInteractive::Fullscreen();

	int left_size = 20;
	int right_size = 20;
	int top_size = 10;
	int bottom_size = 10;

	auto middle = Renderer([] {return text("Middle") | center;});
	auto left = Renderer([&]() { return text("Left: " + std::to_string(left_size)) | center;});
	auto right = Renderer([&]() { return text("Right: " + std::to_string(right_size)) | center;});
	auto top = Renderer([&]() {
		auto table = Table({
			{"Metric", "Value"},
			{"Total", std::to_string(ram_stats.total_bytes)},
			{"Used", std::to_string(ram_stats.used_bytes)},
			{"Available", std::to_string(ram_stats.available_bytes)},
			{"Usage", std::to_string(ram_stats.usage_percentage) + "%"},
		});
		table.SelectColumn(0).Decorate(color(Color::MagentaLight));
		table.SelectColumn(0).Border(ftxui::DASHED);
		table.SelectColumn(1).Decorate(color(Color::LightGreen));
		table.SelectRow(0).Decorate(bold);
		table.SelectAll().Border(ftxui::DOUBLE);
		return table.Render() | frame | border;
	});
	auto bottom = Renderer([&]() { return text("Bottom: " + std::to_string(bottom_size)) | center;});

	auto container = middle;
	container = ResizableSplitLeft(left, container, &left_size);
	container = ResizableSplitRight(right, container, &right_size);
	container = ResizableSplitTop(top, container, &top_size);
	container = ResizableSplitBottom(bottom, container, &bottom_size);

	screen.Loop(container);

	return 0;
}
