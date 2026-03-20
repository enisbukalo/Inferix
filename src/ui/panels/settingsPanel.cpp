#include "settingsPanel.h"
#include "configManager.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace ftxui;

static std::string formatFloat(float value, int precision = 2)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(precision) << value;
	return oss.str();
}

/// Helper: renders a labelled row for text inputs.
/// Label fixed left, input stretches middle-to-right.
static Element settingRowComponent(const std::string &label,
								   Element componentRender)
{
	return hbox(
		{ text(label) | color(Color::MagentaLight), componentRender });
}

/// Helper: slider row — label fixed left, slider in the middle, value right.
static Element sliderRow(const std::string &label,
						 Component slider,
						 const std::string &valueStr)
{
	return hbox({ text(label) | color(Color::MagentaLight),
				  hbox({ slider->Render(),
						 text(valueStr) | color(Color::CyanLight) }) });
}

/// Helper: toggle/checkbox row — label left, toggle pushed right.
static Element toggleRow(const std::string &label, Element componentRender)
{
	return hbox({
		text(label) | color(Color::MagentaLight),
		componentRender,
	});
}

// ---------------------------------------------------------------------------

SettingsPanel::SettingsPanel()
{
	loadFromConfig();
}

void SettingsPanel::loadFromConfig()
{
	auto &cfg = ConfigManager::instance().getConfig();

	// Server
	m_host = cfg.server.host;
	m_port = std::to_string(cfg.server.port);
	m_apiKey = cfg.server.apiKey;
	m_timeout = cfg.server.timeout;
	m_threadsHttp = cfg.server.threadsHttp;
	m_webui = cfg.server.webui;
	m_embedding = cfg.server.embedding;
	m_contBatching = cfg.server.contBatching;
	m_cachePrompt = cfg.server.cachePrompt;
	m_metrics = cfg.server.metrics;

	// Load
	m_modelPath = cfg.load.modelPath;
	m_ngpuLayers = cfg.load.ngpuLayers;
	m_ctxSize = cfg.load.ctxSize == 0 ? "" : std::to_string(cfg.load.ctxSize);
	m_batchSize = cfg.load.batchSize;
	// Flash attention dropdown index
	if (cfg.load.flashAttn == "on")
		m_flashAttnIdx = 1;
	else if (cfg.load.flashAttn == "off")
		m_flashAttnIdx = 2;
	else
		m_flashAttnIdx = 0;
	m_mmap = cfg.load.mmap;
	m_mlock = cfg.load.mlock;
	m_fit = cfg.load.fit;

	// Inference
	m_temperature = static_cast<float>(cfg.inference.temperature);
	m_topP = static_cast<float>(cfg.inference.topP);
	m_topK = cfg.inference.topK;
	m_minP = static_cast<float>(cfg.inference.minP);
	m_repeatPenalty = static_cast<float>(cfg.inference.repeatPenalty);
	m_presencePenalty = static_cast<float>(cfg.inference.presencePenalty);
	m_frequencyPenalty = static_cast<float>(cfg.inference.frequencyPenalty);
	m_nPredict = std::to_string(cfg.inference.nPredict);

	// UI
	auto themeIt =
		std::find(m_themeOptions.begin(), m_themeOptions.end(), cfg.ui.theme);
	m_themeIdx = (themeIt != m_themeOptions.end())
					 ? static_cast<int>(themeIt - m_themeOptions.begin())
					 : 0;
	m_defaultTabIdx = std::clamp(cfg.ui.defaultTab, 0, 2);
	m_showSystemPanel = cfg.ui.showSystemPanel;
	m_refreshRateMs = cfg.ui.refreshRateMs;

	// Terminal
	m_defaultShell = cfg.terminal.defaultShell;
	m_initialCommand = cfg.terminal.initialCommand;
	m_workingDirectory = cfg.terminal.workingDirectory;
	m_defaultCols = cfg.terminal.defaultCols;
	m_defaultRows = cfg.terminal.defaultRows;
}

void SettingsPanel::saveConfig()
{
	auto &cfg = ConfigManager::instance().getConfig();

	// Server
	cfg.server.host = m_host;
	try {
		cfg.server.port = std::stoi(m_port);
	} catch (...) {
	}
	cfg.server.apiKey = m_apiKey;
	cfg.server.timeout = m_timeout;
	cfg.server.threadsHttp = m_threadsHttp;
	cfg.server.webui = m_webui;
	cfg.server.embedding = m_embedding;
	cfg.server.contBatching = m_contBatching;
	cfg.server.cachePrompt = m_cachePrompt;
	cfg.server.metrics = m_metrics;

	// Load
	cfg.load.modelPath = m_modelPath;
	cfg.load.ngpuLayers = m_ngpuLayers;
	try {
		cfg.load.ctxSize = m_ctxSize.empty() ? 0 : std::stoi(m_ctxSize);
	} catch (...) {
	}
	cfg.load.batchSize = m_batchSize;
	cfg.load.flashAttn = m_flashAttnOptions[static_cast<size_t>(m_flashAttnIdx)];
	cfg.load.mmap = m_mmap;
	cfg.load.mlock = m_mlock;
	cfg.load.fit = m_fit;

	// Inference
	cfg.inference.temperature = static_cast<double>(m_temperature);
	cfg.inference.topP = static_cast<double>(m_topP);
	cfg.inference.topK = m_topK;
	cfg.inference.minP = static_cast<double>(m_minP);
	cfg.inference.repeatPenalty = static_cast<double>(m_repeatPenalty);
	cfg.inference.presencePenalty = static_cast<double>(m_presencePenalty);
	cfg.inference.frequencyPenalty = static_cast<double>(m_frequencyPenalty);
	try {
		cfg.inference.nPredict = std::stoi(m_nPredict);
	} catch (...) {
	}

	// UI
	cfg.ui.theme = m_themeOptions[static_cast<size_t>(m_themeIdx)];
	cfg.ui.defaultTab = m_defaultTabIdx;
	cfg.ui.showSystemPanel = m_showSystemPanel;
	cfg.ui.refreshRateMs = m_refreshRateMs;

	// Terminal
	cfg.terminal.defaultShell = m_defaultShell;
	cfg.terminal.initialCommand = m_initialCommand;
	cfg.terminal.workingDirectory = m_workingDirectory;
	cfg.terminal.defaultCols = m_defaultCols;
	cfg.terminal.defaultRows = m_defaultRows;

	ConfigManager::instance().save();
}

Component SettingsPanel::component()
{
	if (m_component)
		return m_component;

	auto onChange = [this] { saveConfig(); };

	// -----------------------------------------------------------------------
	// Server components
	// -----------------------------------------------------------------------
	InputOption inputOpt;
	inputOpt.on_change = onChange;
	inputOpt.multiline = false;

	auto hostInput = Input(&m_host, "127.0.0.1", inputOpt);
	auto portInput = Input(&m_port, "8080", inputOpt);

	InputOption apiKeyOpt = inputOpt;
	apiKeyOpt.password = true;
	auto apiKeyInput = Input(&m_apiKey, "API Key", apiKeyOpt);

	SliderOption<int> timeoutOpt;
	timeoutOpt.value = &m_timeout;
	timeoutOpt.min = 1;
	timeoutOpt.max = 3600;
	timeoutOpt.increment = 10;
	timeoutOpt.on_change = onChange;
	auto timeoutSlider = Slider(timeoutOpt);

	SliderOption<int> threadsHttpOpt;
	threadsHttpOpt.value = &m_threadsHttp;
	threadsHttpOpt.min = -1;
	threadsHttpOpt.max = 64;
	threadsHttpOpt.increment = 1;
	threadsHttpOpt.on_change = onChange;
	auto threadsHttpSlider = Slider(threadsHttpOpt);

	auto webuiCb = Checkbox("Web UI", &m_webui);
	auto embeddingCb = Checkbox("Embedding Mode", &m_embedding);
	auto contBatchCb = Checkbox("Continuous Batching", &m_contBatching);
	auto cachePromptCb = Checkbox("Cache Prompt", &m_cachePrompt);
	auto metricsCb = Checkbox("Metrics", &m_metrics);

	// Checkbox on_change: use CatchEvent wrapper
	// Actually, simpler: use CheckboxOption
	CheckboxOption cbOpt;
	cbOpt.on_change = onChange;
	webuiCb = Checkbox("Web UI", &m_webui, cbOpt);
	embeddingCb = Checkbox("Embedding Mode", &m_embedding, cbOpt);
	contBatchCb = Checkbox("Continuous Batching", &m_contBatching, cbOpt);
	cachePromptCb = Checkbox("Cache Prompt", &m_cachePrompt, cbOpt);
	metricsCb = Checkbox("Metrics", &m_metrics, cbOpt);

	// -----------------------------------------------------------------------
	// Load components
	// -----------------------------------------------------------------------
	auto modelPathInput = Input(&m_modelPath, "path/to/model.gguf", inputOpt);
	auto gpuLayersInput = Input(&m_ngpuLayers, "auto", inputOpt);
	auto ctxSizeInput = Input(&m_ctxSize, "0 = default", inputOpt);

	SliderOption<int> batchSizeOpt;
	batchSizeOpt.value = &m_batchSize;
	batchSizeOpt.min = 32;
	batchSizeOpt.max = 8192;
	batchSizeOpt.increment = 32;
	batchSizeOpt.on_change = onChange;
	auto batchSizeSlider = Slider(batchSizeOpt);

	auto flashAttnToggle = Toggle(&m_flashAttnOptions, &m_flashAttnIdx);

	auto mmapCb = Checkbox("Memory Map", &m_mmap, cbOpt);
	auto mlockCb = Checkbox("Memory Lock", &m_mlock, cbOpt);
	auto fitCb = Checkbox("Fit to Memory", &m_fit, cbOpt);

	// -----------------------------------------------------------------------
	// Inference components
	// -----------------------------------------------------------------------
	SliderOption<float> tempOpt;
	tempOpt.value = &m_temperature;
	tempOpt.min = 0.0f;
	tempOpt.max = 2.0f;
	tempOpt.increment = 0.01f;
	tempOpt.on_change = onChange;
	auto tempSlider = Slider(tempOpt);

	SliderOption<float> topPOpt;
	topPOpt.value = &m_topP;
	topPOpt.min = 0.0f;
	topPOpt.max = 1.0f;
	topPOpt.increment = 0.01f;
	topPOpt.on_change = onChange;
	auto topPSlider = Slider(topPOpt);

	SliderOption<int> topKOpt;
	topKOpt.value = &m_topK;
	topKOpt.min = 0;
	topKOpt.max = 200;
	topKOpt.increment = 1;
	topKOpt.on_change = onChange;
	auto topKSlider = Slider(topKOpt);

	SliderOption<float> minPOpt;
	minPOpt.value = &m_minP;
	minPOpt.min = 0.0f;
	minPOpt.max = 1.0f;
	minPOpt.increment = 0.01f;
	minPOpt.on_change = onChange;
	auto minPSlider = Slider(minPOpt);

	SliderOption<float> repeatPenOpt;
	repeatPenOpt.value = &m_repeatPenalty;
	repeatPenOpt.min = 1.0f;
	repeatPenOpt.max = 2.0f;
	repeatPenOpt.increment = 0.01f;
	repeatPenOpt.on_change = onChange;
	auto repeatPenSlider = Slider(repeatPenOpt);

	SliderOption<float> presPenOpt;
	presPenOpt.value = &m_presencePenalty;
	presPenOpt.min = -2.0f;
	presPenOpt.max = 2.0f;
	presPenOpt.increment = 0.01f;
	presPenOpt.on_change = onChange;
	auto presPenSlider = Slider(presPenOpt);

	SliderOption<float> freqPenOpt;
	freqPenOpt.value = &m_frequencyPenalty;
	freqPenOpt.min = -2.0f;
	freqPenOpt.max = 2.0f;
	freqPenOpt.increment = 0.01f;
	freqPenOpt.on_change = onChange;
	auto freqPenSlider = Slider(freqPenOpt);

	auto nPredictInput = Input(&m_nPredict, "-1 = unlimited", inputOpt);

	// -----------------------------------------------------------------------
	// UI components
	// -----------------------------------------------------------------------
	auto themeToggle = Toggle(&m_themeOptions, &m_themeIdx);
	auto defaultTabToggle = Toggle(&m_tabOptions, &m_defaultTabIdx);
	auto showSysPanelCb =
		Checkbox("Show System Panel", &m_showSystemPanel, cbOpt);

	SliderOption<int> refreshOpt;
	refreshOpt.value = &m_refreshRateMs;
	refreshOpt.min = 50;
	refreshOpt.max = 1000;
	refreshOpt.increment = 10;
	refreshOpt.on_change = onChange;
	auto refreshSlider = Slider(refreshOpt);

	// -----------------------------------------------------------------------
	// Terminal components
	// -----------------------------------------------------------------------
	auto shellInput = Input(&m_defaultShell, "system default", inputOpt);
	auto initCmdInput = Input(&m_initialCommand, "none", inputOpt);
	auto workDirInput = Input(&m_workingDirectory, "current", inputOpt);

	SliderOption<int> colsOpt;
	colsOpt.value = &m_defaultCols;
	colsOpt.min = 16;
	colsOpt.max = 300;
	colsOpt.increment = 1;
	colsOpt.on_change = onChange;
	auto colsSlider = Slider(colsOpt);

	SliderOption<int> rowsOpt;
	rowsOpt.value = &m_defaultRows;
	rowsOpt.min = 8;
	rowsOpt.max = 100;
	rowsOpt.increment = 1;
	rowsOpt.on_change = onChange;
	auto rowsSlider = Slider(rowsOpt);

	// -----------------------------------------------------------------------
	// Container — two-column layout
	// -----------------------------------------------------------------------
	auto container = Container::Horizontal({
		Container::Vertical({
			// Left column: Server, UI, Terminal
			hostInput,		   portInput,	  apiKeyInput, timeoutSlider,
			threadsHttpSlider, webuiCb,		  embeddingCb, contBatchCb,
			cachePromptCb,	   metricsCb,	  themeToggle, defaultTabToggle,
			showSysPanelCb,	   refreshSlider, shellInput,  initCmdInput,
			workDirInput,	   colsSlider,	  rowsSlider,
		}),
		Container::Vertical({
			// Right column: Load, Inference
			modelPathInput,
			gpuLayersInput,
			ctxSizeInput,
			batchSizeSlider,
			flashAttnToggle,
			mmapCb,
			mlockCb,
			fitCb,
			tempSlider,
			topPSlider,
			topKSlider,
			minPSlider,
			repeatPenSlider,
			presPenSlider,
			freqPenSlider,
			nPredictInput,
		}),
	});

	m_component = Renderer(container, [=, this] {
		// === Left column: Server, UI, Terminal ===
		Elements leftElements;

		// Server Settings
		{
			Elements rows;
			rows.push_back(settingRowComponent("Host", hostInput->Render()));
			rows.push_back(settingRowComponent("Port", portInput->Render()));
			rows.push_back(
				settingRowComponent("API Key", apiKeyInput->Render()));
			rows.push_back(sliderRow("Timeout",
									 timeoutSlider,
									 std::to_string(m_timeout) + "s"));
			rows.push_back(sliderRow(
				"HTTP Threads",
				threadsHttpSlider,
				m_threadsHttp == -1 ? "auto" : std::to_string(m_threadsHttp)));
			rows.push_back(toggleRow("", webuiCb->Render()));
			rows.push_back(toggleRow("", embeddingCb->Render()));
			rows.push_back(toggleRow("", contBatchCb->Render()));
			rows.push_back(toggleRow("", cachePromptCb->Render()));
			rows.push_back(toggleRow("", metricsCb->Render()));
			leftElements.push_back(
				window(text("Server Settings") | bold,
					   hbox({ text("    "), vbox(std::move(rows)) }) | flex,
					   ftxui::EMPTY));
		}

		// UI Settings
		{
			Elements rows;
			rows.push_back(toggleRow("Theme", themeToggle->Render()));
			rows.push_back(toggleRow("Default Tab", defaultTabToggle->Render()));
			rows.push_back(toggleRow("", showSysPanelCb->Render()));
			rows.push_back(sliderRow("Refresh Rate",
									 refreshSlider,
									 std::to_string(m_refreshRateMs) + "ms"));
			leftElements.push_back(
				window(text("UI Settings") | bold,
					   hbox({ text("    "), vbox(std::move(rows)) }) | flex,
					   ftxui::EMPTY));
		}

		// Terminal Settings
		{
			Elements rows;
			rows.push_back(
				settingRowComponent("Default Shell", shellInput->Render()));
			rows.push_back(
				settingRowComponent("Initial Command", initCmdInput->Render()));
			rows.push_back(settingRowComponent("Working Directory",
											   workDirInput->Render()));
			rows.push_back(sliderRow("Default Cols",
									 colsSlider,
									 std::to_string(m_defaultCols)));
			rows.push_back(sliderRow("Default Rows",
									 rowsSlider,
									 std::to_string(m_defaultRows)));
		filler(),
			leftElements.push_back(
				window(text("Terminal Settings") | bold,
					   hbox({ text("    "), vbox(std::move(rows)) }) | flex,
					   ftxui::EMPTY));
		}

		// === Right column: Load, Inference ===
		Elements rightElements;

		// Load Settings
		{
			Elements rows;
			rows.push_back(
				settingRowComponent("Model Path", modelPathInput->Render()));
			rows.push_back(
				settingRowComponent("GPU Layers", gpuLayersInput->Render()));
			rows.push_back(
				settingRowComponent("Context Size", ctxSizeInput->Render()));
			rows.push_back(sliderRow("Batch Size",
									 batchSizeSlider,
									 std::to_string(m_batchSize)));
			rows.push_back(
				toggleRow("Flash Attention", flashAttnToggle->Render()));
			rows.push_back(toggleRow("", mmapCb->Render()));
			rows.push_back(toggleRow("", mlockCb->Render()));
			rows.push_back(toggleRow("", fitCb->Render()));
			rightElements.push_back(
				window(text("Load Settings") | bold,
					   hbox({ text("    "), vbox(std::move(rows)) }) | flex,
					   ftxui::EMPTY));
		}

		// Inference Settings
		{
			Elements rows;
			rows.push_back(sliderRow("Temperature",
									 tempSlider,
									 formatFloat(m_temperature)));
			rows.push_back(sliderRow("Top P", topPSlider, formatFloat(m_topP)));
			rows.push_back(
				sliderRow("Top K", topKSlider, std::to_string(m_topK)));
			rows.push_back(sliderRow("Min P", minPSlider, formatFloat(m_minP)));
			rows.push_back(sliderRow("Repeat Penalty",
									 repeatPenSlider,
									 formatFloat(m_repeatPenalty)));
			rows.push_back(sliderRow("Presence Penalty",
									 presPenSlider,
									 formatFloat(m_presencePenalty)));
			rows.push_back(sliderRow("Frequency Penalty",
									 freqPenSlider,
									 formatFloat(m_frequencyPenalty)));
			rows.push_back(
				settingRowComponent("Max Tokens", nPredictInput->Render()));
			rightElements.push_back(
				window(text("Inference Settings") | bold,
					   hbox({ text("    "), vbox(std::move(rows)) }) | flex,
					   ftxui::EMPTY));
		}

		FlexboxConfig colCfg;
		colCfg.direction = FlexboxConfig::Direction::Row;
		colCfg.justify_content = FlexboxConfig::JustifyContent::SpaceBetween;

		auto leftCol = vbox(std::move(leftElements)) | flex;
		auto rightCol = vbox(std::move(rightElements)) | flex;

		return hbox({ leftCol, separatorLight(), rightCol });
	});

	return m_component;
}
