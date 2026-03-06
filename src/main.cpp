/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Minimal main function that delegates all functionality to App::Run().
 * This file serves as the compilation entry point for the Inferix TUI.
 */

#include "app.h"

/**
 * @brief Program entry point.
 *
 * Launches the Inferix terminal UI by calling App::Run() and returns
 * when the application exits.
 *
 * @return 0 on successful execution.
 */
int main() {
	App::Run();
	return 0;
}
