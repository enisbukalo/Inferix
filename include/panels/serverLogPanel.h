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
 * content arrives. Reads from the log file.
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
	 * @brief Clear all log lines.
	 */
	void clear();

  private:
	/**
	 * @brief Render the log content.
	 */
	ftxui::Element renderLog();

	/**
	 * @brief Polling loop to read new lines from log file.
	 */
	void pollLogFile();

	// FTXUI references
	ftxui::ScreenInteractive &m_screen;

	// Log content
	std::vector<std::string> m_lines;
	std::mutex m_linesMutex;

	// Scroll state
	float m_scrollX = 0.0f;
	float m_scrollY = 1.0f;
	ftxui::Box m_box;

	// File polling
	std::string m_logPath;
	std::atomic<bool> m_running{ false };
	std::thread m_pollThread;
	size_t m_lastLineCount{ 0 }; // Track how many lines we've read

	// Component
	ftxui::Component m_component;
};