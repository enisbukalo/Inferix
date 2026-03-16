/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Minimal main function that delegates all functionality to App::Run().
 * This file serves as the compilation entry point for the Workbench TUI.
 */

#include "app.h"
#include "configManager.h"

/**
 * @brief Program entry point.
 *
 * Loads user configuration and launches the Workbench terminal UI.
 *
 * @return 0 on successful execution.
 */
int main()
{
	ConfigManager::instance().load();
	App::run();
	return 0;
}
