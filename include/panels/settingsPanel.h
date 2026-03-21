#pragma once

#include "terminalPresetsPanel.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

/**
 * @file settingsPanel.h
 * @brief Comprehensive stateful settings panel for all application configuration.
 *
 * This is the main configuration hub for Workbench, providing a unified interface
 * to manage all aspects of the application. Unlike ModelsPanel which focuses only
 * on Load+Inference parameters, this panel covers five major sections:
 *
 * **Server Settings** - Network and HTTP server behavior:
 *   Host/port binding, API key authentication, request timeout (60-3600s),
 *   HTTP worker threads (-1=auto or 1-64), feature flags for Web UI,
 *   embedding mode, continuous batching, prompt caching, metrics collection.
 *
 * **Load Settings** - Model loading parameters (partially duplicated from
 *   ModelsPanel):
 *   Model path, GPU layer offloading (-1=auto or 0-N), context size and batch
 *   size controls, flash attention toggle (auto/on/off), memory mapping/locking.
 *
 * **Inference Settings** - Token generation parameters (partially duplicated from
 *   ModelsPanel):
 *   Temperature, top-P/top-K sampling, min-P filtering, repeat/presence/frequency
 *   penalties, max tokens prediction (-1=unlimited).
 *
 * **UI Settings** - Application appearance and behavior:
 *   Theme selection (default/dark/light/monokai), default startup tab,
 *   system panel visibility toggle, refresh rate control (50-1000ms).
 *
 * **Terminal Settings** - Embedded terminal emulator defaults:
 *   Default shell command, initial command to execute on spawn, working
 *   directory, terminal dimensions (cols: 16-300, rows: 8-100).
 *
 * The panel uses a two-column layout with Server/UI settings stacked vertically
 * on the left, and Terminal settings plus an embedded TerminalPresetsPanel on
 * the right. It is stateful - member variables are loaded from ConfigManager in
 * the constructor and persisted immediately via onChange callbacks whenever any
 * setting changes.
 *
 * @note There is intentional duplication with ModelsPanel for Load+Inference
 *       sections, allowing both panels to exist independently without tight
 *       coupling. Future refactoring may consolidate these into a single source.
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
	bool m_metrics = false;

	// --- Load state ---
	std::string m_modelPath;
	std::string m_ngpuLayers;
	std::string m_ctxSize;
	int m_batchSize = 2048;
	std::string m_batchSizeStr = std::to_string(m_batchSize);
	int m_flashAttnIdx = 0; // dropdown index: 0=auto, 1=on, 2=off
	bool m_mmap = true;
	bool m_mlock = false;
	bool m_fit = true;

	// --- Inference state ---
	float m_temperature = 0.8f;
	std::string m_temperatureStr;
	float m_topP = 0.95f;
	std::string m_topPStr;
	int m_topK = 40;
	std::string m_topKStr = std::to_string(m_topK);
	float m_minP = 0.05f;
	std::string m_minPStr;
	float m_repeatPenalty = 1.0f;
	std::string m_repeatPenaltyStr;
	float m_presencePenalty = 0.0f;
	std::string m_presencePenaltyStr;
	float m_frequencyPenalty = 0.0f;
	std::string m_frequencyPenaltyStr;
	std::string m_nPredict;

	// --- UI state ---
	int m_themeIdx = 0;		 // dropdown index
	int m_defaultTabIdx = 0; // dropdown index
	bool m_showSystemPanel = true;
	int m_refreshRateMs = 250;
	std::string m_refreshRateMsStr = std::to_string(m_refreshRateMs);

	// --- Terminal state ---
	std::string m_defaultShell;
	std::string m_initialCommand;
	std::string m_workingDirectory;
	int m_defaultCols = 80;
	std::string m_defaultColsStr = std::to_string(m_defaultCols);
	int m_defaultRows = 24;
	std::string m_defaultRowsStr = std::to_string(m_defaultRows);

	// Dropdown option lists (must be stable for FTXUI references)
	std::vector<std::string> m_flashAttnOptions = { "auto", "on", "off" };
	std::vector<std::string> m_themeOptions = { "default",
												"dark",
												"light",
												"monokai" };
	std::vector<std::string> m_tabOptions = { "Settings",
											  "Server Log",
											  "Terminal" };

	ftxui::Component m_component;
};
