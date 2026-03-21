#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>

namespace ui_utils {

/**
 * @brief Format a float value to string with specified precision
 *
 * @param value The float value to format
 * @param precision Decimal places (default: 2)
 * @return Formatted string representation
 */
std::string formatFloat(float value, int precision = 2);

/**
 * @brief Render a labelled row for text inputs
 *
 * Creates a horizontal box with magenta label on left and component on right.
 *
 * @param label The label text (rendered in MagentaLight)
 * @param componentRender The rendered FTXUI component element
 * @return Element representing the setting row
 */
ftxui::Element settingRowComponent(const std::string &label,
                                   ftxui::Element componentRender);

/**
 * @brief Render a labelled number input row with +/- controls
 *
 * Creates: [Label] [-] [input] [+] layout with magenta label.
 *
 * @param label The label text (rendered in MagentaLight)
 * @param minusBtn The rendered minus button element
 * @param inputRender The rendered input field element
 * @param plusBtn The rendered plus button element
 * @return Element representing the number row with controls
 */
ftxui::Element numberRow(const std::string &label,
                         ftxui::Element minusBtn,
                         ftxui::Element inputRender,
                         ftxui::Element plusBtn);

/**
 * @brief Render a checkbox/toggle row
 *
 * Creates magenta label on left with component on right.
 *
 * @param label The label text (rendered in MagentaLight)
 * @param componentRender The rendered checkbox/toggle element
 * @return Element representing the checkbox row
 */
ftxui::Element checkboxRow(const std::string &label, ftxui::Element componentRender);

} // namespace ui_utils
