/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Minimal main function that delegates all functionality to App::Run().
 * This file serves as the compilation entry point for the Inferix TUI.
 */

#include "app.h"
#include "vterm.h"

/**
 * @brief Program entry point.
 *
 * Launches the Inferix terminal UI by calling App::Run() and returns
 * when the application exits.
 *
 * @return 0 on successful execution.
 */
int main()
{
	VTerm *vt = vterm_new(50, 100);
	VTermScreen *vts = vterm_obtain_screen(vt);
	vterm_screen_reset(vts, 1);
	App::Run();
	return 0;
}
