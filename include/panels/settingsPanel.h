#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

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
