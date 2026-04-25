#include "settingsPanel.h"

#include "eventBus.h"
#include "ui_utils.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace ftxui;

// ---------------------------------------------------------------------------

SettingsPanel::SettingsPanel(AppDependencies &deps) : m_config(deps.config)
{
	loadFromConfig();
}

void SettingsPanel::loadFromConfig()
{
	auto &cfg = m_config.getConfig();

	// Server
	m_executablePath = cfg.server.executablePath;
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
	m_verboseApiLogs = cfg.server.verboseApiLogs;

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
	m_logRetentionDays = cfg.ui.logRetentionDays;
	m_logRetentionDaysStr = std::to_string(m_logRetentionDays);

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
	auto &cfg = m_config.getConfig();

	// ========================================================================
	// Capture old values for change detection
	// ========================================================================
	int oldRefreshRateMs = cfg.ui.refreshRateMs;
	std::string oldTheme = cfg.ui.theme;
	int oldDefaultTab = cfg.ui.defaultTab;
	bool oldShowSystemPanel = cfg.ui.showSystemPanel;

	// ========================================================================
	// Update Server settings
	// ========================================================================
	cfg.server.executablePath = m_executablePath;
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
	cfg.server.verboseApiLogs = m_verboseApiLogs;

	// ========================================================================
	// Update UI settings
	// ========================================================================
	cfg.ui.theme = m_themeOptions[static_cast<size_t>(m_themeIdx)];
	cfg.ui.defaultTab = m_defaultTabIdx;
	cfg.ui.showSystemPanel = m_showSystemPanel;
	cfg.ui.refreshRateMs = m_refreshRateMs;
	cfg.ui.logRetentionDays = m_logRetentionDays;

	// ========================================================================
	// Update Terminal settings
	// ========================================================================
	cfg.terminal.defaultShell = m_defaultShell;
	cfg.terminal.initialCommand = m_initialCommand;
	cfg.terminal.workingDirectory = m_workingDirectory;
	cfg.terminal.defaultCols = m_defaultCols;
	cfg.terminal.defaultRows = m_defaultRows;

	// ========================================================================
	// Publish events for changed values (dynamic updates)
	// ========================================================================
	// UI refresh rate — SystemMonitorRunner subscribes to this
	if (oldRefreshRateMs != cfg.ui.refreshRateMs) {
		EventBus::publish("config.ui.refreshRateMs", &cfg.ui.refreshRateMs);
		spdlog::info("Config changed: ui.refreshRateMs = {}",
					 cfg.ui.refreshRateMs);
	}

	// UI theme — future: ThemeManager could subscribe
	if (oldTheme != cfg.ui.theme) {
		EventBus::publish("config.ui.theme", &cfg.ui.theme);
		spdlog::info("Config changed: ui.theme = {}", cfg.ui.theme);
	}

	// Default tab — future: TabManager could subscribe
	if (oldDefaultTab != cfg.ui.defaultTab) {
		EventBus::publish("config.ui.defaultTab", &cfg.ui.defaultTab);
		spdlog::info("Config changed: ui.defaultTab = {}", cfg.ui.defaultTab);
	}

	// System panel visibility — future: UI could subscribe
	if (oldShowSystemPanel != cfg.ui.showSystemPanel) {
		EventBus::publish("config.ui.showSystemPanel", &cfg.ui.showSystemPanel);
		spdlog::info("Config changed: ui.showSystemPanel = {}",
					 cfg.ui.showSystemPanel);
	}

	// ========================================================================
	// Persist to disk
	// ========================================================================
	m_config.save();
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
	// Creates a triplet of components: decrement button, text input, increment
	// button Value is clamped to [minVal, maxVal] range on every change
	// Immediately persists to config via onChange callback
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

	// -----------------------------------------------------------------------
	// Server components
	// Creates FTXUI input components for server configuration
	// API key uses password mode for privacy
	// -----------------------------------------------------------------------
	auto exePathInput =
		Input(&m_executablePath, "path/to/llama-server", inputOpt);
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
	auto verboseApiLogsCb = Checkbox("", &m_verboseApiLogs, cbOpt);

	// -----------------------------------------------------------------------
	// UI components
	// Creates controls for appearance and behavior settings
	// Theme and default tab use toggle menus for selection
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

	auto [retentionMinus, retentionInput, retentionPlus] =
		makeIntControls(m_logRetentionDays, m_logRetentionDaysStr, 0, 365, 1);

	// -----------------------------------------------------------------------
	// Terminal components
	// Creates inputs for embedded terminal emulator configuration
	// Shell, command, and working directory are free-text inputs
	// Cols/rows use integer controls with reasonable bounds
	// -----------------------------------------------------------------------
	auto shellInput = Input(&m_defaultShell, "system default", inputOpt);
	auto initCmdInput = Input(&m_initialCommand, "none", inputOpt);
	auto workDirInput = Input(&m_workingDirectory, "current", inputOpt);

	auto [colsMinus, colsInput, colsPlus] =
		makeIntControls(m_defaultCols, m_defaultColsStr, 16, 300, 1);
	auto [rowsMinus, rowsInput, rowsPlus] =
		makeIntControls(m_defaultRows, m_defaultRowsStr, 8, 100, 1);

	// -----------------------------------------------------------------------
	// Container — two-column layout (Server/UI left, Terminal right)
	// -----------------------------------------------------------------------
	auto container = Container::Horizontal({
		Container::Vertical({
			// Left column: Server, UI
			exePathInput,	  hostInput,	   portInput,	  apiKeyInput,
			timeoutMinus,	  timeoutInput,	   timeoutPlus,	  threadsHttpMinus,
			threadsHttpInput, threadsHttpPlus, webuiCb,		  embeddingCb,
			contBatchCb,	  cachePromptCb,   themeToggle,	  defaultTabToggle,
			showSysPanelCb,	  refreshMinus,	   refreshInput,  refreshPlus,
			retentionMinus,	  retentionInput,  retentionPlus,
		}),
		Container::Vertical({
			// Right column: Terminal
			shellInput,
			initCmdInput,
			workDirInput,
			colsMinus,
			colsInput,
			colsPlus,
			rowsMinus,
			rowsInput,
			rowsPlus,
		}),
	});

	m_component = Renderer(container, [=, this] {
		// === Left column: Server, UI, Terminal ===
		Elements leftElements;

		// Server Settings
		{
			Elements rows;
			rows.push_back(
				ui_utils::settingRowComponent("Executable Path",
											  exePathInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Host", hostInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Port", portInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("API Key", apiKeyInput->Render()));
			rows.push_back(ui_utils::numberRow("Timeout",
											   timeoutMinus->Render(),
											   timeoutInput->Render(),
											   timeoutPlus->Render()));
			rows.push_back(ui_utils::numberRow("HTTP Threads",
											   threadsHttpMinus->Render(),
											   threadsHttpInput->Render(),
											   threadsHttpPlus->Render()));
			rows.push_back(ui_utils::checkboxRow("Web UI", webuiCb->Render()));
			rows.push_back(
				ui_utils::checkboxRow("Embedding Mode", embeddingCb->Render()));
			rows.push_back(ui_utils::checkboxRow("Continuous Batching",
												 contBatchCb->Render()));
			rows.push_back(
				ui_utils::checkboxRow("Cache Prompt", cachePromptCb->Render()));
			rows.push_back(ui_utils::checkboxRow("Verbose API Logs",
												 verboseApiLogsCb->Render()));
			leftElements.push_back(
				window(text("Server Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		// UI Settings
		{
			Elements rows;
			rows.push_back(
				ui_utils::checkboxRow("Theme", themeToggle->Render()));
			rows.push_back(ui_utils::checkboxRow("Default Tab",
												 defaultTabToggle->Render()));
			rows.push_back(ui_utils::checkboxRow("Show System Panel",
												 showSysPanelCb->Render()));
			rows.push_back(ui_utils::numberRow("System Info Refresh Rate",
											   refreshMinus->Render(),
											   refreshInput->Render(),
											   refreshPlus->Render()));
			rows.push_back(ui_utils::numberRow("Log Retention (days)",
											   retentionMinus->Render(),
											   retentionInput->Render(),
											   retentionPlus->Render()));
			leftElements.push_back(
				window(text("UI Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		// === Right column: Terminal ===
		Elements rightElements;
		{
			Elements rows;
			rows.push_back(ui_utils::settingRowComponent("Default Shell",
														 shellInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Initial Command",
											  initCmdInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Working Directory",
											  workDirInput->Render()));
			rows.push_back(ui_utils::numberRow("Default Cols",
											   colsMinus->Render(),
											   colsInput->Render(),
											   colsPlus->Render()));
			rows.push_back(ui_utils::numberRow("Default Rows",
											   rowsMinus->Render(),
											   rowsInput->Render(),
											   rowsPlus->Render()));
			rightElements.push_back(
				window(text("Terminal Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}
		rightElements.push_back(TerminalPresetsPanel::render(m_config));

		auto leftCol = vbox(std::move(leftElements)) | flex;
		auto rightCol = vbox(std::move(rightElements)) | flex;

		return hbox({ leftCol, separatorLight(), rightCol });
	});

	return m_component;
}
