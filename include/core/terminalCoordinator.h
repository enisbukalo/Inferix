/**
 * @file terminalCoordinator.h
 * @brief Manages static + dynamic terminal panels: creation, spawning, capture state.
 *
 * Encapsulates the logic for:
 * - Creating dynamic terminals from config presets (called once during setup)
 * - Eagerly spawning all terminals before the event loop starts
 * - Checking whether any active terminal tab is capturing input (based on selectedTab index)
 * - Delegating keyboard events and Ctrl-C to the correct panel
 *
 * @note Tab index ordering: Static terminal is always at index 3. Dynamic terminals
 *       are appended at indices 4, 5, 6, etc. This ordering is assumed by all methods.
 */

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

class TerminalPanel;

namespace Config {
struct TerminalPreset;
} // namespace Config

namespace ftxui {
class ComponentBase;
class Event;
class ScreenInteractive;
} // namespace ftxui

/**
 * @brief Plain data structure holding a dynamic terminal panel and metadata.
 * @note This is a POD struct; member naming conventions (m_ prefix) do not apply.
 */
struct DynamicTerminal
{
	std::unique_ptr<TerminalPanel> panel;
	int tabIndex;
	std::string presetName;
};

/**
 * @brief Manages static + dynamic terminal panels: creation, spawning, capture state.
 *
 * Encapsulates the logic for:
 * - Creating dynamic terminals from config presets (called once during setup)
 * - Eagerly spawning all terminals before the event loop starts
 * - Checking whether any active terminal tab is capturing input (based on selectedTab index)
 * - Delegating keyboard events and Ctrl-C to the correct panel
 *
 * @note The coordinator does NOT own the static terminal — it holds a non-owning reference.
 *       Dynamic terminals are owned via std::unique_ptr stored in m_dynamicTerminals.
 */
class TerminalCoordinator
{
  public:
	/**
	 * @brief Construct with the static "Terminal" tab's panel and screen reference.
	 * @param staticTerminal Reference to the main terminal panel (tab index 3).
	 * @param screen FTXUI screen used for creating dynamic terminal panels.
	 * @note The coordinator does NOT own the static terminal; it is owned by the caller.
	 */
	TerminalCoordinator(TerminalPanel &staticTerminal,
						ftxui::ScreenInteractive &screen);

	/**
	 * @brief Create dynamic terminals from config presets and return their components.
	 *
	 * Each preset becomes a new terminal panel. This method returns:
	 * - A vector of FTXUI components (one per dynamic terminal)
	 * - A vector of tab labels (preset names) to be appended to the tab toggle
	 *
	 * The caller is responsible for:
	 * - Adding the components to the Container::Tab
	 * - Appending the labels to the tabValues vector
	 * - Calling spawnAll() before the event loop starts
	 *
	 * @param presets Vector of terminal presets from config.
	 * @return std::pair<std::vector<std::shared_ptr<ftxui::ComponentBase>>, std::vector<std::string>>
		 *         First: components for each dynamic terminal
		 *         Second: corresponding tab labels (preset names)
		 */
	[[nodiscard]]
	std::pair<std::vector<std::shared_ptr<ftxui::ComponentBase>>, std::vector<std::string>>
	createDynamicTerminals(const std::vector<Config::TerminalPreset> &presets);

	/**
	 * @brief Eagerly spawn all terminals (static + dynamic) before the event loop.
	 */
	void spawnAll();

	/**
	 * @brief Check whether any terminal at the given tabIndex is capturing input.
	 * @param tabIndex Current selected tab index.
	 * @return true if the active terminal panel has capture mode enabled.
	 */
	[[nodiscard]] bool isActiveTabCapturing(int tabIndex) const;

	/**
	 * @brief Forward Ctrl-C (ETX byte \x03) to the active terminal's PTY.
	 * @param tabIndex Current selected tab index.
	 * @return true if a terminal consumed the event, false otherwise.
	 */
	bool forwardCtrlC(int tabIndex);

	/**
	 * @brief Delegate an FTXUI event to the active terminal panel for handling.
	 * @param tabIndex Current selected tab index.
	 * @param event The FTXUI event to delegate.
	 * @return true if a terminal handled the event, false otherwise.
	 */
	bool handleTerminalEvent(int tabIndex, ftxui::Event event);

	/**
	 * @brief Auto-capture when switching to a terminal tab.
	 *
	 * Called from the outer Renderer on every tab change. If the newly selected
	 * tab is a terminal (static or dynamic), enables capture mode so keyboard
	 * input flows directly to that PTY. Non-terminal tabs are ignored.
	 *
	 * @param newTabIndex The index of the newly selected tab.
	 */
	void autoCaptureOnTabSwitch(int newTabIndex);

  private:
	ftxui::ScreenInteractive &m_screen;
	TerminalPanel &m_staticTerminal;
	std::vector<DynamicTerminal> m_dynamicTerminals;
};
