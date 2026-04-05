/**
 * @file serverLogPanel.cpp
 * @brief Implementation of scrollable log panel for llama-server output.
 */

#include "serverLogPanel.h"
#include "llamaServerProcess.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

using namespace ftxui;

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

ServerLogPanel::ServerLogPanel(ftxui::ScreenInteractive &screen)
	: m_screen(screen), m_logPath(LlamaServerProcess::getLogPath())
{
	// Component will be created on first access to component()
}

ServerLogPanel::~ServerLogPanel()
{
	m_running.store(false);
	if (m_pollThread.joinable()) {
		m_pollThread.join();
	}
}

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

		// Start polling thread
		m_running.store(true);
		m_pollThread = std::thread(&ServerLogPanel::pollLogFile, this);
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
		return window(text("Server Log"), text("No log output yet...") | dim) |
			   flex;
	}

	return window(text("Server Log"), vbox(std::move(elements)) | flex) | flex;
}

// ---------------------------------------------------------------------------
// Log file polling
// ---------------------------------------------------------------------------

void ServerLogPanel::pollLogFile()
{
	std::ifstream file;

	while (m_running.load()) {
		// Open file if not open
		if (!file.is_open()) {
			file.open(m_logPath, std::ios::in);
			if (!file.is_open()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				continue;
			}
			// Seek to end to only read new content
			file.seekg(0, std::ios::end);
			m_lastReadPos.store(file.tellg());
		}

		// Read new lines
		std::string line;
		bool newContent = false;

		while (std::getline(file, line)) {
			if (!line.empty()) {
				{
					std::lock_guard<std::mutex> lock(m_linesMutex);
					m_lines.push_back(line);
				}
				newContent = true;
			}
		}

		// Update position and trigger redraw if new content
		if (file.good()) {
			m_lastReadPos.store(file.tellg());
		} else {
			// Clear fail state and reopen
			file.close();
		}

		if (newContent) {
			m_screen.PostEvent(Event::Custom);
		}

		// Sleep before next poll
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
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