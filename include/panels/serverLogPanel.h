#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/**
 * @file serverLogPanel.h
 * @brief Scrollable log panel for displaying llama-server output.
 *
 * Displays log output from llama-server in a scrollable window with
 * horizontal and vertical sliders. Auto-scrolls to bottom when new
 * content arrives.
 */
class ServerLogPanel
{
  public:
	/**
	 * @brief Construct a ServerLogPanel.
	 *
	 * @param screen Reference to FTXUI screen for posting redraws.
	 */
	explicit ServerLogPanel(ftxui::ScreenInteractive &screen);

	/**
	 * @brief Destructor - stops polling thread.
	 */
	~ServerLogPanel();

	// Non-copyable
	ServerLogPanel(const ServerLogPanel &) = delete;
	ServerLogPanel &operator=(const ServerLogPanel &) = delete;

	/**
	 * @brief Get the FTXUI component for this panel.
	 * @return Component suitable for adding to a container.
	 */
	ftxui::Component component();

	/**
	 * @brief Append a line to the log (called from LlamaServerProcess).
	 * @param line The line to append.
	 */
	void appendLine(const std::string &line);

	/**
	 * @brief Clear all log lines.
	 */
	void clear();

  private:
	/**
	 * @brief Render the log content.
	 */
	ftxui::Element renderLog();

	// FTXUI references
	ftxui::ScreenInteractive &m_screen;

	// Log content
	std::vector<std::string> m_lines;
	std::mutex m_linesMutex;

	// Scroll state
	float m_scrollX = 0.0f;
	float m_scrollY = 1.0f; // Start at bottom for auto-scroll
	ftxui::Box m_box;

	// Component
	ftxui::Component m_component;
};