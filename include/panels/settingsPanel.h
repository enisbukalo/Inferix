#pragma once

#include "eventBus.h"
#include "terminalPresetsPanel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

/**
 * @file settingsPanel.h
 * @brief Stateful settings panel for server, UI, and terminal configuration.
 *
 * This is the main configuration hub for Workbench non-model settings,
 * providing a unified interface to manage server behavior, application
 * appearance, and embedded terminal defaults.
 *
 * **Server Settings** - Network and HTTP server behavior:
 *   Host/port binding, API key authentication, request timeout (1-3600s),
 *   HTTP worker threads (-1=auto or 1-64), feature flags for Web UI,
 *   embedding mode, continuous batching, prompt caching, metrics collection.
 *
 * **UI Settings** - Application appearance and behavior:
 *   Theme selection (default/dark/light/monokai), default startup tab,
 *   system panel visibility toggle, refresh rate control (50-1000ms).
 *
 * **Terminal Settings** - Embedded terminal emulator defaults:
 *   Default shell command, initial command to execute on spawn, working
 *   directory, terminal dimensions (cols: 16-300, rows: 8-100).
 *
 * **Note**: Load and Inference settings have been moved to ModelsPanel to
 * avoid duplication. SettingsPanel now focuses exclusively on server,
 * UI, and terminal configuration.
 *
 * The panel uses a two-column layout with Server/UI settings stacked
 * vertically on the left, and Terminal settings plus an embedded
 * TerminalPresetsPanel on the right. It is stateful — member variables
 * are loaded from ConfigManager in the constructor and persisted
 * immediately via onChange callbacks whenever any setting changes.
 */
class SettingsPanel
{
  public:
	SettingsPanel();

	ftxui::Component component();

  private:
	void loadFromConfig();
	void saveConfig();

	// --- Server state ---
	std::string m_executablePath;
	std::string m_host;
	std::string m_port;
	std::string m_apiKey;
	int m_timeout = 600;
	std::string m_timeoutStr = std::to_string(m_timeout);
	int m_threadsHttp = -1;
	std::string m_threadsHttpStr = std::to_string(m_threadsHttp);
	bool m_webui = true;
	bool m_embedding = false;
	bool m_contBatching = true;
	bool m_cachePrompt = true;
	// m_metrics removed - metrics is always enabled

	// Verbose API logging - off by default, shows /models, /metrics, /slots in
	// logs
	bool m_verboseApiLogs = false;

	// --- UI state ---
	int m_themeIdx = 0;		 // dropdown index
	int m_defaultTabIdx = 0; // dropdown index
	bool m_showSystemPanel = true;
	int m_refreshRateMs = 250;
	std::string m_refreshRateMsStr = std::to_string(m_refreshRateMs);
	int m_logRetentionDays = 7;
	std::string m_logRetentionDaysStr = std::to_string(m_logRetentionDays);

	// --- Terminal state ---
	std::string m_defaultShell;
	std::string m_initialCommand;
	std::string m_workingDirectory;
	int m_defaultCols = 80;
	std::string m_defaultColsStr = std::to_string(m_defaultCols);
	int m_defaultRows = 24;
	std::string m_defaultRowsStr = std::to_string(m_defaultRows);

	// Dropdown option lists (must be stable for FTXUI references)
	std::vector<std::string> m_themeOptions = { "default",
												"dark",
												"light",
												"monokai" };
	std::vector<std::string> m_tabOptions = { "Settings",
											  "Server Log",
											  "Terminal" };

	ftxui::Component m_component;
};
