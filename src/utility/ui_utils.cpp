#include "ui_utils.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include <iomanip>
#include <sstream>

namespace ui_utils {

using namespace ftxui;

std::string formatFloat(float value, int precision)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(precision) << value;
	return oss.str();
}

Element settingRowComponent(const std::string &label, Element componentRender)
{
	return hbox({ text(label) | color(Color::MagentaLight),
				  filler(),
				  componentRender }) |
		   xflex;
}

Element numberRow(const std::string &label,
				  Element minusBtn,
				  Element inputRender,
				  Element plusBtn)
{
	return hbox({ text(label) | color(Color::MagentaLight) | vcenter,
				  filler(),
				  minusBtn,
				  separatorLight(),
				  inputRender | size(WIDTH, EQUAL, 8),
				  separatorLight(),
				  plusBtn }) |
		   xflex;
}

Element checkboxRow(const std::string &label, Element componentRender)
{
	return hbox({ text(label) | color(Color::MagentaLight),
				  filler(),
				  componentRender }) |
		   xflex;
}

} // namespace ui_utils
