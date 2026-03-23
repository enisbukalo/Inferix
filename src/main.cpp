/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Minimal main function that delegates all functionality to App::Run().
 * This file serves as the compilation entry point for the Workbench TUI.
 */

#include "app.h"
#include "configManager.h"
#include "systemMonitorRunner.h"

/**
 * @brief Program entry point.
 *
 * Loads user configuration, starts the system monitor singleton, and
 * launches the Workbench terminal UI.
 *
 * @return 0 on successful execution.
 */
int main()
{
	// Load configuration from disk
	ConfigManager::instance().load();
	
	// Start the system monitor singleton with the configured refresh rate
	auto& config = ConfigManager::instance().getConfig();
	SystemMonitorRunner::instance().start(config.ui.refreshRateMs);
	
	// Run the main application UI loop
	App::run();
	
	// Clean up the system monitor before exit
	SystemMonitorRunner::instance().stop();
	
	return 0;
}
