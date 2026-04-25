/**
 * @file terminalCoordinator.cpp
 * @brief Implementation of TerminalCoordinator — terminal lifecycle management.
 */

#include "terminalCoordinator.h"

#include <memory>
#include <utility>

#include "terminalPanel.h"
#include "userSettings.h" // For TerminalPreset

TerminalCoordinator::TerminalCoordinator(TerminalPanel &staticTerminal,
										 ftxui::ScreenInteractive &screen)
	: m_screen(screen), m_staticTerminal(staticTerminal)
{
}

std::pair<std::vector<std::shared_ptr<ftxui::ComponentBase>>,
		  std::vector<std::string>>
TerminalCoordinator::createDynamicTerminals(
	const std::vector<Config::TerminalPreset> &presets)
{
	std::vector<std::shared_ptr<ftxui::ComponentBase>> components;
	std::vector<std::string> labels;

	// Start dynamic terminal index at 4 (0: App Settings, 1: Model Settings,
	// 2: Server Log, 3: Static Terminal).
	int nextIndex = 4;
	for (const auto &preset : presets) {
		auto panel =
			std::make_unique<TerminalPanel>(m_screen, preset.initialCommand);
		auto component = panel->component();

		labels.push_back(preset.name);
		components.push_back(std::move(component));
		m_dynamicTerminals.push_back(
			{ std::move(panel), nextIndex, preset.name });
		nextIndex++;
	}

	return { components, labels };
}

void TerminalCoordinator::spawnAll()
{
	m_staticTerminal.spawn();
	for (auto &dt : m_dynamicTerminals) {
		dt.panel->spawn();
	}
}

bool TerminalCoordinator::isActiveTabCapturing(int tabIndex) const
{
	if (tabIndex == 3 && m_staticTerminal.isCapturing()) {
		return true;
	}
	for (const auto &dt : m_dynamicTerminals) {
		if (tabIndex == dt.tabIndex && dt.panel->isCapturing()) {
			return true;
		}
	}
	return false;
}

bool TerminalCoordinator::forwardCtrlC(int tabIndex)
{
	if (tabIndex == 3 && m_staticTerminal.isCapturing()) {
		m_staticTerminal.sendCtrlC();
		return true;
	}
	for (auto &dt : m_dynamicTerminals) {
		if (tabIndex == dt.tabIndex && dt.panel->isCapturing()) {
			dt.panel->sendCtrlC();
			return true;
		}
	}
	return false;
}

bool TerminalCoordinator::handleTerminalEvent(int tabIndex, ftxui::Event event)
{
	if (tabIndex == 3 && m_staticTerminal.wantsEvent(event)) {
		return m_staticTerminal.handleEvent(event);
	}
	for (auto &dt : m_dynamicTerminals) {
		if (tabIndex == dt.tabIndex && dt.panel->wantsEvent(event)) {
			return dt.panel->handleEvent(event);
		}
	}
	return false;
}

void TerminalCoordinator::autoCaptureOnTabSwitch(int newTabIndex)
{
	if (newTabIndex == 3) {
		m_staticTerminal.setCapturing(true);
	}
	for (auto &dt : m_dynamicTerminals) {
		if (newTabIndex == dt.tabIndex) {
			dt.panel->setCapturing(true);
		}
	}
}
