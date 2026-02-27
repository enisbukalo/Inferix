#include "system_info.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

Element SystemInfo::Render() {
    auto table = Table({
        {"CPU",   "AMD 5800x"},
        {"GPU 0", "Nvidia 5060ti"},
        {"GPU 1", "Nvidia 5060ti"},
    });
    table.SelectColumn(0).Decorate(color(Color::Cyan));
    table.SelectColumn(1).Decorate(color(Color::LightGreen));
    table.SelectColumn(0).Border(ftxui::DASHED);

    return table.Render();
}
