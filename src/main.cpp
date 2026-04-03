/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Minimal main function that delegates all functionality to App::Run().
 * This file serves as the compilation entry point for the Workbench TUI.
 */

#include "app.h"
#include "configManager.h"
#include "llamaServerProcess.h"
#include "systemMonitorRunner.h"

/**
 * @brief Program entry point.
 *
 * Loads user configuration and launches the Workbench terminal UI.
 * The SystemMonitorRunner is started from within App::run() after the
 * screen is created, so it can trigger redraws when monitor data updates.
 *
 * @return 0 on successful execution.
 */
int main()
{
	// Load configuration from disk
	ConfigManager::instance().load();

	// Run the main application UI loop
	App::run();

	// Clean up the system monitor before exit
	SystemMonitorRunner::instance().stop();

	// Terminate llama-server if still running
	LlamaServerProcess::instance().terminate();

	return 0;
}
