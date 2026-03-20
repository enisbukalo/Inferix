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
	int m_threadsHttp = -1;
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
	int m_flashAttnIdx = 0; // dropdown index: 0=auto, 1=on, 2=off
	bool m_mmap = true;
	bool m_mlock = false;
	bool m_fit = true;

	// --- Inference state ---
	float m_temperature = 0.8f;
	float m_topP = 0.95f;
	int m_topK = 40;
	float m_minP = 0.05f;
	float m_repeatPenalty = 1.0f;
	float m_presencePenalty = 0.0f;
	float m_frequencyPenalty = 0.0f;
	std::string m_nPredict;

	// --- UI state ---
	int m_themeIdx = 0;		 // dropdown index
	int m_defaultTabIdx = 0; // dropdown index
	bool m_showSystemPanel = true;
	int m_refreshRateMs = 250;

	// --- Terminal state ---
	std::string m_defaultShell;
	std::string m_initialCommand;
	std::string m_workingDirectory;
	int m_defaultCols = 80;
	int m_defaultRows = 24;

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
