/**
 * @file tabLayout.h
 * @brief Assembles the complete UI layout: SystemResourcesPanel + separator +
 *        tabbed Toggle/Container + green-border capture indicator + ServerInfoPanel footer.
 *
 * Wraps everything in a single Renderer that:
 * - Renders system resource gauges at the top (delegates to SystemResourcesPanel::render)
 * - Draws an orange-3 separator line
 * - Renders the tabbed toggle + container area with conditional green border when capturing
 * - Renders server status footer (delegates to ServerInfoPanel::render)
 *
 * Takes monitor references (ICpuMonitor, IMemoryMonitor, IGpuMonitor, IModelInfoMonitor)
 * and a server reference (ILlamaServerProcess) via the AppDependencies pattern.
 */

#pragma once

#include <string>
#include <vector>

class TerminalCoordinator;

namespace ftxui {
class Component;
} // namespace ftxui

/**
 * @brief Assembles the complete UI layout: SystemResourcesPanel + separator +
 *        tabbed Toggle/Container + green-border capture indicator + ServerInfoPanel footer.
 */
class TabLayout
{
  public:
	/**
	 * @brief Construct with all pre-built components and dependencies.
	 * @param selectedTab Pointer to the currently selected tab index — Toggle writes here.
	 * @param settingsComponent Settings panel's FTXUI component.
	 * @param modelComponent Models panel's FTXUI component.
	 * @param logOutputComponent Server log panel's FTXUI component.
	 * @param terminalComponent Terminal panel's FTXUI component.
	 * @param dynamicTerminalComponents Pre-built components for dynamic terminal tabs.
	 * @param dynamicTabLabels Tab labels corresponding to each dynamic terminal.
	 * @param coordinator Reference to TerminalCoordinator for capture-state queries + auto-capture.
	 * @param cpu CPU monitor reference (passed through to SystemResourcesPanel::render).
	 * @param mem Memory monitor reference (passed through to SystemResourcesPanel::render).
	 * @param gpu GPU monitor reference (passed through to SystemResourcesPanel::render).
	 * @param modelInfo Model info monitor reference (passed through to SystemResourcesPanel::render).
	 * @param server Llama server reference (passed through to ServerInfoPanel::render).
	 */
	TabLayout(int *selectedTab,
			  ftxui::Component settingsComponent,
			  ftxui::Component modelComponent,
			  ftxui::Component logOutputComponent,
			  ftxui::Component terminalComponent,
			  std::vector<ftxui::Component> dynamicTerminalComponents,
			  std::vector<std::string> dynamicTabLabels,
			  TerminalCoordinator &coordinator,
			  class ICpuMonitor &cpu,
			  class IMemoryMonitor &mem,
			  class IGpuMonitor &gpu,
			  class IModelInfoMonitor &modelInfo,
			  class ILlamaServerProcess &server);

	/**
	 * @brief Build the complete outer layout wrapped in a Renderer.
	 *
	 * Creates:
	 *   1. Toggle with static + dynamic tab labels
	 *   2. All tab components (static + dynamic), each wrapped in Renderer(window(...))
	 *   3. Container::Tab holding all content panels
	 *   4. Outer Renderer that composes on every frame:
	 *      a. SystemResourcesPanel::render(cpu, mem, gpu, modelInfo) — top gauges
	 *      b. separatorCharacter("*") | bold | color(Color::Orange3) — divider line
	 *      c. Container::Vertical(Toggle, TabContainer) with conditional green border
	 *         when coordinator.isActiveTabCapturing() returns true
	 *      d. ServerInfoPanel::render(server) — footer status bar
	 *   5. All wrapped in vbox(...) | flex
	 *
	 * The Renderer also calls coordinator.autoCaptureOnTabSwitch(selectedTab) on tab changes,
	 * so switching to a terminal tab automatically enables capture mode.
	 *
	 * @return The fully assembled component ready for CatchEvent wrapping by EventRouter.
	 */
	ftxui::Component build();

  private:
	int *m_selectedTab;
	ftxui::Component m_settingsComponent;
	ftxui::Component m_modelComponent;
	ftxui::Component m_logOutputComponent;
	ftxui::Component m_terminalComponent;
	std::vector<ftxui::Component> m_dynamicTerminalComponents;
	std::vector<std::string> m_dynamicTabLabels;
	TerminalCoordinator &m_coordinator;

	// Monitor references — passed through to panel render calls.
	class ICpuMonitor &m_cpu;
	class IMemoryMonitor &m_mem;
	class IGpuMonitor &m_gpu;
	class IModelInfoMonitor &m_modelInfo;
	class ILlamaServerProcess &m_server;

	int m_prevTab = -1; // Track previous tab for auto-capture on switch.

	/**
	 * @brief Wrap a component in a Renderer with window + flex styling.
	 */
	static ftxui::Component wrapInWindow(ftxui::Component comp);
};
