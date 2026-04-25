/**
 * @file app.cpp
 * @brief Main application implementation.
 *
 * Implements the App class that orchestrates the Workbench TUI layout
 * and initializes the background monitoring system. Thin wiring function
 * that delegates to TerminalCoordinator, TabLayout, and EventRouter.
 */

#include "app.h"
#include "appDependencies.h"
#include "configManager.h"
#include "cpuMonitor.h"
#include "eventRouter.h"
#include "gpuMonitor.h"
#include "llamaServerProcess.h"
#include "modelInfoMonitor.h"
#include "modelsIni.h"
#include "modelsPanel.h"
#include "ramMonitor.h"
#include "serverLogPanel.h"
#include "settingsPanel.h"
#include "systemMonitorRunner.h"
#include "tabLayout.h"
#include "terminalCoordinator.h"
#include "terminalPanel.h"

#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <string>

void App::run()
{
	spdlog::info("App::run() - initializing");

	// Clear llama-server log on startup (truncate existing file)
	{
		std::string logPath = ConfigManager::getLogsDir() + "/llama-server.log";
		std::ofstream(logPath, std::ios::trunc).close();
	}

	auto screen = ftxui::ScreenInteractive::Fullscreen();

	// Disable FTXUI's forced Ctrl-C handling. By default FTXUI generates
	// SIGABRT for Ctrl-C if unhandled. We intercept it ourselves and forward
	// the ETX byte (\x03) to the PTY so the shell process gets SIGINT.
	screen.ForceHandleCtrlC(false);

	// Load config and start the system monitor singleton with the screen
	// reference. This allows it to trigger redraws when monitor data updates.
	auto &config = ConfigManager::instance().getConfig();
	SystemMonitorRunner::instance().start(&screen, config.ui.refreshRateMs);

	// Build dependency injection struct from singleton instances (Issue #1 pattern).
	AppDependencies deps{
		ConfigManager::instance(),		// IConfigManager&
		LlamaServerProcess::instance(), // ILlamaServerProcess&
		ModelInfoMonitor::instance(),	// IModelInfoMonitor&
		ModelsIni::instance(),			// IModelsIni&
		CpuMonitor::instance(),			// ICpuMonitor&
		MemoryMonitor::instance(),		// IMemoryMonitor&
		GpuMonitor::instance()			// IGpuMonitor&
	};

	// Construct panels with dependency injection.
	TerminalPanel terminalPanel(screen);
	SettingsPanel settingsPanel(deps);
	ModelsPanel modelsPanel(deps);
	ServerLogPanel serverLogPanel(screen);

	// Start the server log watcher PTY before entering the event loop.
	serverLogPanel.start();

	// Create dynamic terminals from config presets and collect their components.
	TerminalCoordinator coordinator(terminalPanel, screen);
	auto [dynamicComponents, dynamicTabLabels] =
		coordinator.createDynamicTerminals(config.terminalPresets);
	spdlog::info("Spawned {} terminal preset(s)", dynamicComponents.size());

	// Build the full UI layout (SystemResourcesPanel + tabs + ServerInfoPanel footer).
	int selectedTab = 0;
	TabLayout layout(&selectedTab,
					 settingsPanel.component(),
					 modelsPanel.component(),
					 serverLogPanel.component(),
					 terminalPanel.component(),
					 std::move(dynamicComponents),
					 std::move(dynamicTabLabels),
					 coordinator,
					 deps.cpu, deps.mem, deps.gpu, deps.modelInfo, deps.server);

	// Spawn all terminals eagerly so they're ready when the user switches tabs.
	coordinator.spawnAll();

	// Wrap with event routing (Ctrl-C forwarding + keyboard delegation).
	EventRouter router(layout.build(), &selectedTab, coordinator);
	screen.Loop(router.route());

	spdlog::info("App::run() - exiting");
}
