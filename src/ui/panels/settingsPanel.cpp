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
static Element settingRowComponent(const std::string &label,
								   Element componentRender)
{
	return hbox({ text(label) | color(Color::MagentaLight),
				  filler(),
				  componentRender }) |
		   xflex;
}

/// Helper: renders a labelled row with [-] [input] [+] controls.
static Element numberRow(const std::string &label,
						 Element minusBtn,
						 Element inputRender,
						 Element plusBtn)
{
	return hbox({ text(label) | color(Color::MagentaLight) | vcenter,
				  filler(),
				  minusBtn,
				  separatorLight(),
				  inputRender | size(WIDTH, EQUAL, 8),
				  separatorLight(),
				  plusBtn }) |
		   xflex;
}

/// Helper: checkbox/toggle row — magenta label left, component right.
static Element checkboxRow(const std::string &label, Element componentRender)
{
	return hbox({ text(label) | color(Color::MagentaLight),
				  filler(),
				  componentRender }) |
		   xflex;
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
	m_timeoutStr = std::to_string(m_timeout);
	m_threadsHttp = cfg.server.threadsHttp;
	m_threadsHttpStr = std::to_string(m_threadsHttp);
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
	m_batchSizeStr = std::to_string(m_batchSize);
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
	m_temperatureStr = formatFloat(m_temperature);
	m_topP = static_cast<float>(cfg.inference.topP);
	m_topPStr = formatFloat(m_topP);
	m_topK = cfg.inference.topK;
	m_topKStr = std::to_string(m_topK);
	m_minP = static_cast<float>(cfg.inference.minP);
	m_minPStr = formatFloat(m_minP);
	m_repeatPenalty = static_cast<float>(cfg.inference.repeatPenalty);
	m_repeatPenaltyStr = formatFloat(m_repeatPenalty);
	m_presencePenalty = static_cast<float>(cfg.inference.presencePenalty);
	m_presencePenaltyStr = formatFloat(m_presencePenalty);
	m_frequencyPenalty = static_cast<float>(cfg.inference.frequencyPenalty);
	m_frequencyPenaltyStr = formatFloat(m_frequencyPenalty);
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
	m_refreshRateMsStr = std::to_string(m_refreshRateMs);

	// Terminal
	m_defaultShell = cfg.terminal.defaultShell;
	m_initialCommand = cfg.terminal.initialCommand;
	m_workingDirectory = cfg.terminal.workingDirectory;
	m_defaultCols = cfg.terminal.defaultCols;
	m_defaultColsStr = std::to_string(m_defaultCols);
	m_defaultRows = cfg.terminal.defaultRows;
	m_defaultRowsStr = std::to_string(m_defaultRows);
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
	// Shared options
	// -----------------------------------------------------------------------
	auto toggleOnColor = Color::CyanLight;
	auto toggleOffColor = Color::Cyan;

	InputOption inputOpt;
	inputOpt.on_change = onChange;
	inputOpt.multiline = false;
	inputOpt.transform = [=](InputState state) {
		auto e = state.element | align_right;
		if (state.is_placeholder)
			return e | color(toggleOffColor);
		return e | color(toggleOnColor);
	};

	auto btnStyle = ButtonOption::Animated();
	btnStyle.transform = [=](const EntryState &s) {
		auto e = text(s.label) | color(toggleOnColor);
		if (s.focused)
			e |= bold;
		return e | center;
	};

	CheckboxOption cbOpt;
	cbOpt.on_change = onChange;
	cbOpt.transform = [=](const EntryState &s) {
		auto label = s.state ? text("[X]") : text("[ ]");
		if (s.state)
			label |= color(toggleOnColor);
		else
			label |= color(toggleOffColor);
		if (s.focused)
			label |= bold;
		return label;
	};

	auto toggleOpt = MenuOption::Toggle();
	toggleOpt.on_change = onChange;
	toggleOpt.entries_option.transform = [=](const EntryState &s) {
		auto e = text(s.label);
		if (s.active)
			e |= color(toggleOnColor);
		else
			e |= color(toggleOffColor);
		if (s.focused)
			e |= bold;
		return e;
	};

	// Helper: create [-] [input] [+] for an int field
	auto makeIntControls =
		[&](int &value, std::string &str, int minVal, int maxVal, int step) {
			struct Controls
			{
				Component minus, input, plus;
			};
			auto minus = Button(
				"-",
				[&value, &str, minVal, step, onChange] {
					value = std::max(minVal, value - step);
					str = std::to_string(value);
					onChange();
				},
				btnStyle);
			auto plus = Button(
				"+",
				[&value, &str, maxVal, step, onChange] {
					value = std::min(maxVal, value + step);
					str = std::to_string(value);
					onChange();
				},
				btnStyle);
			InputOption numInputOpt = inputOpt;
			numInputOpt.transform = [=](InputState state) {
				auto e = state.element | center;
				if (state.is_placeholder)
					return e | color(toggleOffColor);
				return e | color(toggleOnColor);
			};
			numInputOpt.on_change = [&value, &str, minVal, maxVal, onChange] {
				try {
					int v = std::stoi(str);
					value = std::clamp(v, minVal, maxVal);
				} catch (...) {
				}
				onChange();
			};
			auto inp = Input(&str, "", numInputOpt);
			return Controls{ minus, inp, plus };
		};

	// Helper: create [-] [input] [+] for a float field
	auto makeFloatControls = [&](float &value,
								 std::string &str,
								 float minVal,
								 float maxVal,
								 float step) {
		struct Controls
		{
			Component minus, input, plus;
		};
		auto minus = Button(
			"-",
			[&value, &str, minVal, step, onChange] {
				value = std::max(minVal, value - step);
				str = formatFloat(value);
				onChange();
			},
			btnStyle);
		auto plus = Button(
			"+",
			[&value, &str, maxVal, step, onChange] {
				value = std::min(maxVal, value + step);
				str = formatFloat(value);
				onChange();
			},
			btnStyle);
		InputOption numInputOpt = inputOpt;
		numInputOpt.transform = [=](InputState state) {
			auto e = state.element | center;
			if (state.is_placeholder)
				return e | color(toggleOffColor);
			return e | color(toggleOnColor);
		};
		numInputOpt.on_change = [&value, &str, minVal, maxVal, onChange] {
			try {
				float v = std::stof(str);
				value = std::clamp(v, minVal, maxVal);
			} catch (...) {
			}
			onChange();
		};
		auto inp = Input(&str, "", numInputOpt);
		return Controls{ minus, inp, plus };
	};

	// -----------------------------------------------------------------------
	// Server components
	// -----------------------------------------------------------------------
	auto hostInput = Input(&m_host, "127.0.0.1", inputOpt);
	auto portInput = Input(&m_port, "8080", inputOpt);

	InputOption apiKeyOpt = inputOpt;
	apiKeyOpt.password = true;
	auto apiKeyInput = Input(&m_apiKey, "API Key", apiKeyOpt);

	auto [timeoutMinus, timeoutInput, timeoutPlus] =
		makeIntControls(m_timeout, m_timeoutStr, 1, 3600, 10);
	auto [threadsHttpMinus, threadsHttpInput, threadsHttpPlus] =
		makeIntControls(m_threadsHttp, m_threadsHttpStr, -1, 64, 1);

	auto webuiCb = Checkbox("", &m_webui, cbOpt);
	auto embeddingCb = Checkbox("", &m_embedding, cbOpt);
	auto contBatchCb = Checkbox("", &m_contBatching, cbOpt);
	auto cachePromptCb = Checkbox("", &m_cachePrompt, cbOpt);
	auto metricsCb = Checkbox("", &m_metrics, cbOpt);

	// -----------------------------------------------------------------------
	// Load components
	// -----------------------------------------------------------------------
	auto modelPathInput = Input(&m_modelPath, "path/to/model.gguf", inputOpt);
	auto gpuLayersInput = Input(&m_ngpuLayers, "auto", inputOpt);
	auto ctxSizeInput = Input(&m_ctxSize, "0 = default", inputOpt);

	auto [batchSizeMinus, batchSizeInput, batchSizePlus] =
		makeIntControls(m_batchSize, m_batchSizeStr, 32, 8192, 32);

	auto flashAttnOpt = toggleOpt;
	flashAttnOpt.entries = &m_flashAttnOptions;
	flashAttnOpt.selected = &m_flashAttnIdx;
	auto flashAttnToggle = Menu(flashAttnOpt);
	auto mmapCb = Checkbox("", &m_mmap, cbOpt);
	auto mlockCb = Checkbox("", &m_mlock, cbOpt);
	auto fitCb = Checkbox("", &m_fit, cbOpt);

	// -----------------------------------------------------------------------
	// Inference components
	// -----------------------------------------------------------------------
	auto [tempMinus, tempInput, tempPlus] =
		makeFloatControls(m_temperature, m_temperatureStr, 0.0f, 2.0f, 0.01f);
	auto [topPMinus, topPInput, topPPlus] =
		makeFloatControls(m_topP, m_topPStr, 0.0f, 1.0f, 0.01f);
	auto [topKMinus, topKInput, topKPlus] =
		makeIntControls(m_topK, m_topKStr, 0, 200, 1);
	auto [minPMinus, minPInput, minPPlus] =
		makeFloatControls(m_minP, m_minPStr, 0.0f, 1.0f, 0.01f);
	auto [repeatPenMinus, repeatPenInput, repeatPenPlus] =
		makeFloatControls(m_repeatPenalty,
						  m_repeatPenaltyStr,
						  1.0f,
						  2.0f,
						  0.01f);
	auto [presPenMinus, presPenInput, presPenPlus] =
		makeFloatControls(m_presencePenalty,
						  m_presencePenaltyStr,
						  -2.0f,
						  2.0f,
						  0.01f);
	auto [freqPenMinus, freqPenInput, freqPenPlus] =
		makeFloatControls(m_frequencyPenalty,
						  m_frequencyPenaltyStr,
						  -2.0f,
						  2.0f,
						  0.01f);

	auto nPredictInput = Input(&m_nPredict, "-1 = unlimited", inputOpt);

	// -----------------------------------------------------------------------
	// UI components
	// -----------------------------------------------------------------------
	auto themeOpt = toggleOpt;
	themeOpt.entries = &m_themeOptions;
	themeOpt.selected = &m_themeIdx;
	auto themeToggle = Menu(themeOpt);

	auto defaultTabOpt = toggleOpt;
	defaultTabOpt.entries = &m_tabOptions;
	defaultTabOpt.selected = &m_defaultTabIdx;
	auto defaultTabToggle = Menu(defaultTabOpt);
	auto showSysPanelCb = Checkbox("", &m_showSystemPanel, cbOpt);

	auto [refreshMinus, refreshInput, refreshPlus] =
		makeIntControls(m_refreshRateMs, m_refreshRateMsStr, 50, 1000, 10);

	// -----------------------------------------------------------------------
	// Terminal components
	// -----------------------------------------------------------------------
	auto shellInput = Input(&m_defaultShell, "system default", inputOpt);
	auto initCmdInput = Input(&m_initialCommand, "none", inputOpt);
	auto workDirInput = Input(&m_workingDirectory, "current", inputOpt);

	auto [colsMinus, colsInput, colsPlus] =
		makeIntControls(m_defaultCols, m_defaultColsStr, 16, 300, 1);
	auto [rowsMinus, rowsInput, rowsPlus] =
		makeIntControls(m_defaultRows, m_defaultRowsStr, 8, 100, 1);

	// -----------------------------------------------------------------------
	// Container — two-column layout
	// -----------------------------------------------------------------------
	auto container = Container::Horizontal({
		Container::Vertical({
			// Left column: Server, UI, Terminal
			hostInput,		 portInput,	   apiKeyInput,		 timeoutMinus,
			timeoutInput,	 timeoutPlus,  threadsHttpMinus, threadsHttpInput,
			threadsHttpPlus, webuiCb,	   embeddingCb,		 contBatchCb,
			cachePromptCb,	 metricsCb,	   themeToggle,		 defaultTabToggle,
			showSysPanelCb,	 refreshMinus, refreshInput,	 refreshPlus,
			shellInput,		 initCmdInput, workDirInput,	 colsMinus,
			colsInput,		 colsPlus,	   rowsMinus,		 rowsInput,
			rowsPlus,
		}),
		Container::Vertical({
			// Right column: Load, Inference
			modelPathInput, gpuLayersInput, ctxSizeInput,	 batchSizeMinus,
			batchSizeInput, batchSizePlus,	flashAttnToggle, mmapCb,
			mlockCb,		fitCb,			tempMinus,		 tempInput,
			tempPlus,		topPMinus,		topPInput,		 topPPlus,
			topKMinus,		topKInput,		topKPlus,		 minPMinus,
			minPInput,		minPPlus,		repeatPenMinus,	 repeatPenInput,
			repeatPenPlus,	presPenMinus,	presPenInput,	 presPenPlus,
			freqPenMinus,	freqPenInput,	freqPenPlus,	 nPredictInput,
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
			rows.push_back(numberRow("Timeout",
									 timeoutMinus->Render(),
									 timeoutInput->Render(),
									 timeoutPlus->Render()));
			rows.push_back(numberRow("HTTP Threads",
									 threadsHttpMinus->Render(),
									 threadsHttpInput->Render(),
									 threadsHttpPlus->Render()));
			rows.push_back(checkboxRow("Web UI", webuiCb->Render()));
			rows.push_back(checkboxRow("Embedding Mode", embeddingCb->Render()));
			rows.push_back(
				checkboxRow("Continuous Batching", contBatchCb->Render()));
			rows.push_back(checkboxRow("Cache Prompt", cachePromptCb->Render()));
			rows.push_back(checkboxRow("Metrics", metricsCb->Render()));
			leftElements.push_back(
				window(text("Server Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		// UI Settings
		{
			Elements rows;
			rows.push_back(checkboxRow("Theme", themeToggle->Render()));
			rows.push_back(
				checkboxRow("Default Tab", defaultTabToggle->Render()));
			rows.push_back(
				checkboxRow("Show System Panel", showSysPanelCb->Render()));
			rows.push_back(numberRow("Refresh Rate",
									 refreshMinus->Render(),
									 refreshInput->Render(),
									 refreshPlus->Render()));
			leftElements.push_back(
				window(text("UI Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
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
			rows.push_back(numberRow("Default Cols",
									 colsMinus->Render(),
									 colsInput->Render(),
									 colsPlus->Render()));
			rows.push_back(numberRow("Default Rows",
									 rowsMinus->Render(),
									 rowsInput->Render(),
									 rowsPlus->Render()));
			leftElements.push_back(
				window(text("Terminal Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
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
			rows.push_back(numberRow("Batch Size",
									 batchSizeMinus->Render(),
									 batchSizeInput->Render(),
									 batchSizePlus->Render()));
			rows.push_back(
				checkboxRow("Flash Attention", flashAttnToggle->Render()));
			rows.push_back(checkboxRow("Memory Map", mmapCb->Render()));
			rows.push_back(checkboxRow("Memory Lock", mlockCb->Render()));
			rows.push_back(checkboxRow("Fit to Memory", fitCb->Render()));
			rightElements.push_back(
				window(text("Load Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		// Inference Settings
		{
			Elements rows;
			rows.push_back(numberRow("Temperature",
									 tempMinus->Render(),
									 tempInput->Render(),
									 tempPlus->Render()));
			rows.push_back(numberRow("Top P",
									 topPMinus->Render(),
									 topPInput->Render(),
									 topPPlus->Render()));
			rows.push_back(numberRow("Top K",
									 topKMinus->Render(),
									 topKInput->Render(),
									 topKPlus->Render()));
			rows.push_back(numberRow("Min P",
									 minPMinus->Render(),
									 minPInput->Render(),
									 minPPlus->Render()));
			rows.push_back(numberRow("Repeat Penalty",
									 repeatPenMinus->Render(),
									 repeatPenInput->Render(),
									 repeatPenPlus->Render()));
			rows.push_back(numberRow("Presence Penalty",
									 presPenMinus->Render(),
									 presPenInput->Render(),
									 presPenPlus->Render()));
			rows.push_back(numberRow("Frequency Penalty",
									 freqPenMinus->Render(),
									 freqPenInput->Render(),
									 freqPenPlus->Render()));
			rows.push_back(
				settingRowComponent("Max Tokens", nPredictInput->Render()));
			rightElements.push_back(
				window(text("Inference Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		auto leftCol = vbox(std::move(leftElements)) | flex;
		auto rightCol = vbox(std::move(rightElements)) | flex;

		return hbox({ leftCol, separatorLight(), rightCol });
	});

	return m_component;
}
