/**
 * @file eventRouter.cpp
 * @brief Implementation of EventRouter — keyboard event routing via CatchEvent.
 */

#include "eventRouter.h"

#include <ftxui/component/event.hpp>

#include "terminalCoordinator.h"

using namespace ftxui;

EventRouter::EventRouter(Component component,
						 int *selectedTab,
						 TerminalCoordinator &coordinator)
	: m_component(std::move(component)),
	  m_selectedTab(selectedTab),
	  m_coordinator(coordinator)
{
}

Component EventRouter::route()
{
	return m_component | CatchEvent([this](Event event) {
		// Ctrl+C — forward ETX byte to the active terminal's PTY so the
		// shell's line discipline generates SIGINT for the foreground
		// process group. This prevents FTXUI from quitting the app.
		if (event == Event::CtrlC) {
			return m_coordinator.forwardCtrlC(*m_selectedTab);
		}

		// Delegate other keyboard events to active terminal panel.
		return m_coordinator.handleTerminalEvent(*m_selectedTab, event);
	});
}
