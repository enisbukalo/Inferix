/**
 * @file serverLogPanel.cpp
 * @brief Implementation of scrollable log panel for llama-server output.
 */

#include "serverLogPanel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

ServerLogPanel::ServerLogPanel(ftxui::ScreenInteractive &screen)
	: m_screen(screen)
{
	// Component will be created on first access to component()
}

ServerLogPanel::~ServerLogPanel() = default;

// ---------------------------------------------------------------------------
// Component creation
// ---------------------------------------------------------------------------

Component ServerLogPanel::component()
{
	if (m_component == nullptr) {
		// Create content renderer
		auto content = Renderer([this] { return renderLog(); });

		// Make scrollable with sliders
		auto scrollable = Renderer(content, [&, content] {
			return content->Render() |
				   focusPositionRelative(m_scrollX, m_scrollY) | frame | flex;
		});

		// Horizontal slider
		SliderOption<float> optionX;
		optionX.value = &m_scrollX;
		optionX.min = 0.0f;
		optionX.max = 1.0f;
		optionX.increment = 0.1f;
		optionX.direction = Direction::Right;
		optionX.color_active = Color::Blue;
		optionX.color_inactive = Color::BlueLight;
		auto scrollbarX = Slider(optionX);

		// Vertical slider
		SliderOption<float> optionY;
		optionY.value = &m_scrollY;
		optionY.min = 0.0f;
		optionY.max = 1.0f;
		optionY.increment = 0.1f;
		optionY.direction = Direction::Down;
		optionY.color_active = Color::Yellow;
		optionY.color_inactive = Color::YellowLight;
		auto scrollbarY = Slider(optionY);

		m_component = Container::Vertical({
			Container::Horizontal({
				scrollable | flex,
				scrollbarY,
			}),
			Container::Horizontal({
				scrollbarX,
				Renderer([] { return text(L""); }),
			}),
		});
	}

	return m_component;
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

Element ServerLogPanel::renderLog()
{
	std::lock_guard<std::mutex> lock(m_linesMutex);

	std::vector<Element> elements;
	for (const auto &line : m_lines) {
		elements.push_back(text(line));
	}

	if (elements.empty()) {
		return window(text("Server Log"),
					  text("Waiting for server output...") | dim) |
			   flex;
	}

	return window(text("Server Log"), vbox(std::move(elements)) | flex) | flex;
}

// ---------------------------------------------------------------------------
// Public methods
// ---------------------------------------------------------------------------

void ServerLogPanel::appendLine(const std::string &line)
{
	{
		std::lock_guard<std::mutex> lock(m_linesMutex);
		m_lines.push_back(line);
	}
	m_screen.PostEvent(Event::Custom);
}

void ServerLogPanel::clear()
{
	std::lock_guard<std::mutex> lock(m_linesMutex);
	m_lines.clear();
	m_screen.PostEvent(Event::Custom);
}