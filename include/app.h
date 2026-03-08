#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

/**
 * @brief Main application orchestrator for the Inferix TUI.
 *
 * Manages the application lifecycle and initializes the terminal UI
 * with all panels and the background monitoring system. The application
 * uses a singleton pattern with a static Run() entry point.
 *
 * @note This class is intentionally minimal; all functionality is
 *       delegated to panel classes and the SystemMonitorRunner.
 */
class App {
  public:
	/**
	 * @brief Starts the main application loop.
	 *
	 * Creates an FTXUI fullscreen screen, initializes the background
	 * system monitor (CPU/GPU/RAM polling), and launches the main
	 * UI event loop with all panels arranged in a grid layout.
	 *
	 * This is the primary entry point for the application and blocks
	 * until the user exits the TUI.
	 */
	static void Run();

  private:
	/**
	 * @brief Default constructor - private to prevent instantiation.
	 *
	 * The App class uses only static methods and cannot be instantiated.
	 */
	App() = default;
};
