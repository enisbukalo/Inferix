/**
 * @file tabLayout.cpp
 * @brief Implementation of TabLayout — full outer UI composition.
 */

#include "tabLayout.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "ICpuMonitor.h"
#include "IGpuMonitor.h"
#include "ILlamaServerProcess.h"
#include "IMemoryMonitor.h"
#include "IModelInfoMonitor.h"
#include "serverInfoPanel.h"
#include "systemResourcesPanel.h"
#include "terminalCoordinator.h"

using namespace ftxui;

TabLayout::TabLayout(int *selectedTab,
					 Component settingsComponent,
					 Component modelComponent,
					 Component logOutputComponent,
					 Component terminalComponent,
					 std::vector<Component> dynamicTerminalComponents,
					 std::vector<std::string> dynamicTabLabels,
					 TerminalCoordinator &coordinator,
					 ICpuMonitor &cpu,
					 IMemoryMonitor &mem,
					 IGpuMonitor &gpu,
					 IModelInfoMonitor &modelInfo,
					 ILlamaServerProcess &server)
	: m_selectedTab(selectedTab),
	  m_settingsComponent(std::move(settingsComponent)),
	  m_modelComponent(std::move(modelComponent)),
	  m_logOutputComponent(std::move(logOutputComponent)),
	  m_terminalComponent(std::move(terminalComponent)),
	  m_dynamicTerminalComponents(std::move(dynamicTerminalComponents)),
	  m_dynamicTabLabels(std::move(dynamicTabLabels)),
	  m_coordinator(coordinator),
	  m_cpu(cpu),
	  m_mem(mem),
	  m_gpu(gpu),
	  m_modelInfo(modelInfo),
	  m_server(server)
{
}

Component TabLayout::wrapInWindow(Component comp)
{
	return Renderer(comp, [&] {
		return window(text(""), flex(comp->Render()), EMPTY) | flex;
	});
}

Component TabLayout::build()
{
	// Build tab labels: 4 static + dynamic.
	std::vector<std::string> tabValues{"App Settings", "Model Settings", "Server Log", "Terminal"};
	for (const auto &label : m_dynamicTabLabels) {
		tabValues.push_back(label);
	}

	auto tabToggle = Toggle(&tabValues, m_selectedTab);

	// Build static tab content.
	auto settingsContent = wrapInWindow(m_settingsComponent);
	auto modelContent = wrapInWindow(m_modelComponent);
	auto logOutputContent = wrapInWindow(m_logOutputComponent);
	auto terminalContent = m_terminalComponent;

	// Server info footer bar.
	auto serverContent = Renderer([&] {
		return hbox({text("Some really long status about the server probably "
						 "here."),
					 filler(),
					 ServerInfoPanel::render(m_server)});
	});

	// Collect all tab components: static + dynamic.
	std::vector<Component> allTabComponents{settingsContent, modelContent, logOutputContent, terminalContent};
	for (auto &dynComp : m_dynamicTerminalComponents) {
		allTabComponents.push_back(wrapInWindow(dynComp));
	}

	auto tabContainer = Container::Tab(allTabComponents, m_selectedTab);

	auto interactive = Container::Vertical({tabToggle, tabContainer}) | flex;

	// Outer renderer: composes system resources + separator + tabs + server footer.
	return Renderer(interactive, [&] {
		// Auto-capture on tab switch.
		if (*m_selectedTab != m_prevTab) {
			m_coordinator.autoCaptureOnTabSwitch(*m_selectedTab);
			m_prevTab = *m_selectedTab;
		}

		bool anyCapturing = m_coordinator.isActiveTabCapturing(*m_selectedTab);

		auto panel = interactive->Render() | borderRounded;
		if (anyCapturing) {
			panel = panel | color(Color::LightGreen);
		}

		return vbox({SystemResourcesPanel::render(m_cpu, m_mem, m_gpu, m_modelInfo),
					 separatorCharacter("*") | bold | color(Color::Orange3),
					 panel,
					 serverContent->Render()}) |
			   flex;
	});
}
