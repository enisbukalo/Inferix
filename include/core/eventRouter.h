/**
 * @file eventRouter.h
 * @brief Attaches event interception (CatchEvent) to a FTXUI component hierarchy.
 *
 * Routes keyboard events based on active tab:
 * - Ctrl-C → forward ETX byte (\x03) to the active terminal's PTY
 * - Other events → delegate to TerminalCoordinator for panel-level handling
 */

#pragma once

#include <ftxui/component/component.hpp>

class TerminalCoordinator;

/**
 * @brief Attaches event interception (CatchEvent) to a FTXUI component hierarchy.
 *
 * Routes keyboard events based on active tab:
 * - Ctrl-C → forward ETX byte (\x03) to the active terminal's PTY
 * - Other events → delegate to TerminalCoordinator for panel-level handling
 */
class EventRouter
{
  public:
	/**
	 * @brief Construct with the component hierarchy and a coordinator reference.
	 * @param component The FTXUI component tree to wrap.
	 * @param selectedTab Pointer to current tab index (read-only, updated by Toggle).
	 * @param coordinator Reference to terminal lifecycle + capture state manager.
	 */
	EventRouter(ftxui::Component component,
				int *selectedTab,
				TerminalCoordinator &coordinator);

	/**
	 * @brief Wrap the component in CatchEvent with Ctrl-C forwarding + event delegation.
	 * @return The CatchEvent-wrapped Component ready to be passed to ScreenInteractive::Loop().
	 */
	ftxui::Component route();

  private:
	ftxui::Component m_component;
	int *m_selectedTab;
	TerminalCoordinator &m_coordinator;
};
