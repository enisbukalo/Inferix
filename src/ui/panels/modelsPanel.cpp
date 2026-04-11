#include "modelsPanel.h"
#include "configManager.h"
#include "modelDiscovery.h"
#include "ui_utils.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace ftxui;

/**
 * @brief Find the index of a value in an options vector, or 0 if not found.
 */
static int findOptionIndex(const std::vector<std::string> &options,
						   const std::string &value)
{
	for (size_t i = 0; i < options.size(); ++i)
		if (options[i] == value)
			return static_cast<int>(i);
	return 0;
}

// =========================================================================
// Constructor and Config Methods
// =========================================================================

ModelsPanel::ModelsPanel()
{
	loadFromConfig();
}

void ModelsPanel::loadFromConfig()
{
	auto &cfg = ConfigManager::instance().getConfig();

	spdlog::debug("Loaded settings from config");

	// Load settings
	m_modelPath = cfg.load.modelPath;
	m_ngpuLayers = cfg.load.ngpuLayers;
	m_ctxSize = cfg.load.ctxSize == 0 ? "" : std::to_string(cfg.load.ctxSize);
	m_batchSize = cfg.load.batchSize;
	m_batchSizeStr = std::to_string(m_batchSize);
	m_parallel = cfg.load.parallel;
	m_parallelStr = m_parallel < 0 ? "-1" : std::to_string(m_parallel);

	// Flash attention dropdown index
	if (cfg.load.flashAttn == "on")
		m_flashAttnIdx = 1;
	else if (cfg.load.flashAttn == "off")
		m_flashAttnIdx = 2;
	else
		m_flashAttnIdx = 0;

	m_kvOffload = cfg.load.kvOffload;
	m_kvUnified = cfg.load.kvUnified;
	m_mmap = cfg.load.mmap;
	m_mlock = cfg.load.mlock;
	m_fit = cfg.load.fit;
	m_devicePriority = cfg.load.devicePriority;
	m_splitMode = cfg.load.splitMode;
	m_tensorSplit = cfg.load.tensorSplit;
	m_cacheTypeK = cfg.load.cacheTypeK;
	m_cacheTypeV = cfg.load.cacheTypeV;
	m_lora = cfg.load.lora;
	m_mmproj = cfg.load.mmproj;
	m_modelDraft = cfg.load.modelDraft;
	m_draftMax = std::to_string(cfg.load.draftMax);
	m_chatTemplate = cfg.load.chatTemplate;
	m_reasoningFormat = cfg.load.reasoningFormat;

	// Dropdown indices
	m_splitModeIdx = findOptionIndex(m_splitModeOptions, cfg.load.splitMode);
	m_cacheTypeKIdx = findOptionIndex(m_cacheTypeOptions, cfg.load.cacheTypeK);
	m_cacheTypeVIdx = findOptionIndex(m_cacheTypeOptions, cfg.load.cacheTypeV);
	m_reasoningFormatIdx =
		findOptionIndex(m_reasoningFormatOptions, cfg.load.reasoningFormat);

	// Inference settings
	m_temperature = static_cast<float>(cfg.inference.temperature);
	m_temperatureStr = ui_utils::formatFloat(m_temperature);
	m_topP = static_cast<float>(cfg.inference.topP);
	m_topPStr = ui_utils::formatFloat(m_topP);
	m_topK = cfg.inference.topK;
	m_topKStr = std::to_string(m_topK);
	m_minP = static_cast<float>(cfg.inference.minP);
	m_minPStr = ui_utils::formatFloat(m_minP);
	m_repeatPenalty = static_cast<float>(cfg.inference.repeatPenalty);
	m_repeatPenaltyStr = ui_utils::formatFloat(m_repeatPenalty);
	m_presencePenalty = static_cast<float>(cfg.inference.presencePenalty);
	m_presencePenaltyStr = ui_utils::formatFloat(m_presencePenalty);
	m_frequencyPenalty = static_cast<float>(cfg.inference.frequencyPenalty);
	m_frequencyPenaltyStr = ui_utils::formatFloat(m_frequencyPenalty);
	m_nPredict = std::to_string(cfg.inference.nPredict);
	m_seed = std::to_string(cfg.inference.seed);

	// Refresh model list from models.ini
	refreshModelList();

	// Try to select the model matching config.load.modelPath (if found)
	// Note: config.load.modelPath stores the section name from models.ini
	for (size_t i = 0; i < m_modelNames.size(); ++i) {
		if (m_modelNames[i] == cfg.load.modelPath) {
			m_modelDropdownIndex = static_cast<int>(i);
			m_selectedModelName = m_modelNames[i];
			break;
		}
	}

	// Load presets for the initially selected model
	refreshPresetsForModel();
}

void ModelsPanel::saveConfig()
{
	auto &cfg = ConfigManager::instance().getConfig();

	// Load settings
	cfg.load.modelPath = m_modelPath;
	cfg.load.ngpuLayers = m_ngpuLayers;
	try {
		cfg.load.ctxSize = m_ctxSize.empty() ? 0 : std::stoi(m_ctxSize);
	} catch (...) {
	}
	cfg.load.batchSize = m_batchSize;
	cfg.load.parallel = m_parallel;
	cfg.load.flashAttn = m_flashAttnOptions[static_cast<size_t>(m_flashAttnIdx)];
	cfg.load.kvOffload = m_kvOffload;
	cfg.load.kvUnified = m_kvUnified;
	cfg.load.mmap = m_mmap;
	cfg.load.mlock = m_mlock;
	cfg.load.fit = m_fit;
	cfg.load.devicePriority = m_devicePriority;
	cfg.load.splitMode = m_splitMode;
	cfg.load.tensorSplit = m_tensorSplit;
	cfg.load.cacheTypeK =
		m_cacheTypeOptions[static_cast<size_t>(m_cacheTypeKIdx)];
	cfg.load.cacheTypeV =
		m_cacheTypeOptions[static_cast<size_t>(m_cacheTypeVIdx)];
	cfg.load.lora = m_lora;
	cfg.load.mmproj = m_mmproj;
	cfg.load.modelDraft = m_modelDraft;
	try {
		cfg.load.draftMax = std::stoi(m_draftMax);
	} catch (...) {
	}
	cfg.load.chatTemplate = m_chatTemplate;
	cfg.load.reasoningFormat =
		m_reasoningFormatOptions[static_cast<size_t>(m_reasoningFormatIdx)];

	// Inference settings
	cfg.inference.seed = std::stoi(m_seed);
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

	ConfigManager::instance().save();
	spdlog::info("Model settings saved (model: {})",
				 m_modelPath.empty() ? "<none>" : m_modelPath);
}

// =========================================================================
// Model Discovery Integration
// =========================================================================

/**
 * @brief Check if a model path should be filtered out based on fileFilter
 * patterns.
 *
 * Implements glob-style wildcard matching (case-insensitive):
 * - `mmproj*` → matches filenames starting with "mmproj"
 * - `*mmproj` → matches filenames ending with "mmproj"
 * - `*mmproj*` → matches filenames containing "mmproj"
 * - No `*` → substring match anywhere in filename
 *
 * @param path Full or partial path to the model file
 * @return true if the model should be filtered out (excluded), false otherwise
 */
bool ModelsPanel::shouldFilterModel(const std::string &path) const
{
	// Extract just the filename from the path
	std::string filename = path;
	size_t lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		filename = path.substr(lastSlash + 1);
	}

	// Convert filename to lowercase for case-insensitive matching
	std::string lowerFilename;
	lowerFilename.reserve(filename.size());
	for (char c : filename) {
		lowerFilename +=
			static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}

	// Get filter patterns from config
	auto &cfg = ConfigManager::instance().getConfig();
	const auto &filters = cfg.discovery.fileFilter;

	// Check each filter pattern - if ANY matches, filter out the model
	for (const auto &pattern : filters) {
		if (pattern.empty()) {
			continue;
		}

		// Convert pattern to lowercase
		std::string lowerPattern;
		lowerPattern.reserve(pattern.size());
		for (char c : pattern) {
			lowerPattern +=
				static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		// Find position of '*'
		size_t starPos = lowerPattern.find('*');

		if (starPos == std::string::npos) {
			// No wildcard: substring match
			if (lowerFilename.find(lowerPattern) != std::string::npos) {
				return true;
			}
		} else if (starPos == 0 && lowerPattern.back() == '*') {
			// *pattern*: contains match
			std::string searchStr =
				lowerPattern.substr(1, lowerPattern.size() - 2);
			if (lowerFilename.find(searchStr) != std::string::npos) {
				return true;
			}
		} else if (starPos == 0) {
			// *pattern: ends with
			std::string suffix = lowerPattern.substr(1);
			if (lowerFilename.size() >= suffix.size() &&
				lowerFilename.compare(lowerFilename.size() - suffix.size(),
									  suffix.size(),
									  suffix) == 0) {
				return true;
			}
		} else {
			// pattern*: starts with
			std::string prefix = lowerPattern.substr(0, starPos);
			if (lowerFilename.rfind(prefix, 0) == 0) {
				return true;
			}
		}
	}

	return false;
}

void ModelsPanel::refreshModelList()
{
	auto entries = ModelsIni::instance().getUniqueModelEntries();

	// Sort by display name
	std::sort(
		entries.begin(),
		entries.end(),
		[](const ModelsIni::ModelEntry &a, const ModelsIni::ModelEntry &b) {
			return a.displayName < b.displayName;
		});

	m_modelNames.clear();
	m_modelDisplayNames.clear();
	m_modelPaths.clear();
	for (const auto &entry : entries) {
		m_modelNames.push_back(entry.displayName);
		m_modelDisplayNames.push_back(entry.displayName);
		m_modelPaths.push_back(entry.modelPath);
	}

	// Reset dropdown index if it's now out of bounds
	if (m_modelDropdownIndex < 0 ||
		m_modelDropdownIndex >= static_cast<int>(m_modelNames.size())) {
		m_modelDropdownIndex = 0;
		if (!m_modelNames.empty()) {
			m_selectedModelName = m_modelNames[0];
		}
	}
}

// =========================================================================
// Component Method
// =========================================================================

Component ModelsPanel::component()
{
	if (m_component)
		return m_component;

	// Refresh server state on first creation (server may already be running from
	// app startup)
	refreshServerState();
	updateStartStopLabel();

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

	auto loadBtnStyle = ButtonOption::Animated();
	loadBtnStyle.transform = [=](const EntryState &s) {
		auto e = text(s.label) | color(toggleOnColor);
		if (s.focused)
			e |= bold;
		e |= bgcolor(Color::MagentaLight);
		return e | center;
	};

	// Start/Stop button style - Green text when stopped (LOAD), Red when running
	// (UNLOAD/STOP)
	auto startStopBtnStyle = ButtonOption::Animated();
	startStopBtnStyle.transform = [=](const EntryState &s) {
		Color textColor;
		if (s.label == "STARTING...")
			textColor = Color::YellowLight;
		else if (s.label == "LOAD")
			textColor = Color::GreenLight;
		else
			textColor = Color::RedLight;
		auto e = text(s.label) | color(textColor);
		if (s.focused)
			e |= bold;
		return e | center;
	};

	// Load/Unload button style (magenta LOAD, yellow UNLOAD)
	auto loadUnloadBtnStyle = ButtonOption::Animated();
	loadUnloadBtnStyle.transform = [=](const EntryState &s) {
		auto e = text(s.label) | color(toggleOnColor);
		if (s.focused)
			e |= bold;
		// Magenta for LOAD, Yellow for UNLOAD
		auto label = s.label;
		if (label == "UNLOAD") {
			e |= bgcolor(Color::YellowLight);
		} else {
			e |= bgcolor(Color::MagentaLight);
		}
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

	// Helper: create [-] [input] [+] for a float field
	// Same pattern as makeIntControls but for floating-point values
	// Uses ui_utils::formatFloat for consistent decimal representation
	// Useful for probability/temperature parameters (e.g., temperature, top_p)
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
				str = ui_utils::formatFloat(value);
				onChange();
			},
			btnStyle);
		auto plus = Button(
			"+",
			[&value, &str, maxVal, step, onChange] {
				value = std::min(maxVal, value + step);
				str = ui_utils::formatFloat(value);
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
	// Load components
	// Creates FTXUI input components for model loading parameters
	// Each component is bound to a member variable and triggers auto-save
	// -----------------------------------------------------------------------
	auto gpuLayersInput = Input(&m_ngpuLayers, "auto", inputOpt);
	auto devicePriorityInput = Input(&m_devicePriority, "e.g. 0", inputOpt);
	auto ctxSizeInput = Input(&m_ctxSize, "0 = default", inputOpt);

	auto [batchSizeMinus, batchSizeInput, batchSizePlus] =
		makeIntControls(m_batchSize, m_batchSizeStr, 32, 8192, 32);

	auto [parallelMinus, parallelInput, parallelPlus] =
		makeIntControls(m_parallel, m_parallelStr, -1, 128, 1);

	auto flashAttnOpt = toggleOpt;
	flashAttnOpt.entries = &m_flashAttnOptions;
	flashAttnOpt.selected = &m_flashAttnIdx;
	auto flashAttnToggle = Menu(flashAttnOpt);

	// New: Split mode dropdown
	auto splitModeOpt = toggleOpt;
	splitModeOpt.entries = &m_splitModeOptions;
	splitModeOpt.selected = &m_splitModeIdx;
	auto splitModeToggle = Menu(splitModeOpt);

	// New: Cache type K dropdown
	auto cacheTypeKOpt = toggleOpt;
	cacheTypeKOpt.entries = &m_cacheTypeOptions;
	cacheTypeKOpt.selected = &m_cacheTypeKIdx;
	auto cacheTypeKToggle = Menu(cacheTypeKOpt);

	// New: Cache type V dropdown
	auto cacheTypeVOpt = toggleOpt;
	cacheTypeVOpt.entries = &m_cacheTypeOptions;
	cacheTypeVOpt.selected = &m_cacheTypeVIdx;
	auto cacheTypeVToggle = Menu(cacheTypeVOpt);

	// New: Reasoning format dropdown
	auto reasoningFormatOpt = toggleOpt;
	reasoningFormatOpt.entries = &m_reasoningFormatOptions;
	reasoningFormatOpt.selected = &m_reasoningFormatIdx;
	auto reasoningFormatToggle = Menu(reasoningFormatOpt);

	// New: Text inputs for new settings
	auto tensorSplitInput = Input(&m_tensorSplit, "e.g. 1,1", inputOpt);
	auto loraInput = Input(&m_lora, "path/to/adapter.gguf", inputOpt);
	auto mmprojInput = Input(&m_mmproj, "path/to/mmproj.gguf", inputOpt);
	auto modelDraftInput = Input(&m_modelDraft, "path/to/draft.gguf", inputOpt);
	auto draftMaxInput = Input(&m_draftMax, "-1 = auto", inputOpt);
	auto chatTemplateInput = Input(&m_chatTemplate, "e.g. chatml", inputOpt);

	auto kvOffloadCb = Checkbox("", &m_kvOffload, cbOpt);
	auto kvUnifiedCb = Checkbox("", &m_kvUnified, cbOpt);
	auto mmapCb = Checkbox("", &m_mmap, cbOpt);
	auto mlockCb = Checkbox("", &m_mlock, cbOpt);
	auto fitCb = Checkbox("", &m_fit, cbOpt);

	// -----------------------------------------------------------------------
	// Preset components
	// -----------------------------------------------------------------------
	// Refresh presets for the initially selected model
	refreshPresetsForModel();

	auto presetMenuOpt = MenuOption::Vertical();
	presetMenuOpt.on_change = [this] {
		// Always apply preset when clicked (even if same one re-selected)
		if (m_selectedPresetIndex >= 0 &&
			m_selectedPresetIndex < static_cast<int>(m_presetsForModel.size())) {
			applyPreset(m_presetsForModel[m_selectedPresetIndex]);
			m_editingPresetName = m_presetsForModel[m_selectedPresetIndex].name;
			m_presetStatus.clear();
		} else if (!m_presetsForModel.empty()) {
			// Edge case: index out of bounds but presets exist → select first
			m_selectedPresetIndex = 0;
			applyPreset(m_presetsForModel[0]);
			m_editingPresetName = m_presetsForModel[0].name;
			m_presetStatus.clear();
		}
	};
	presetMenuOpt.entries_option.transform = [=](const EntryState &s) {
		auto e = text(s.label);
		if (s.active)
			e |= color(toggleOnColor);
		else
			e |= color(toggleOffColor);
		if (s.focused)
			e |= bold;
		return e;
	};
	auto presetMenu =
		Menu(&m_presetDisplayNames, &m_selectedPresetIndex, presetMenuOpt);

	InputOption presetNameOpt;
	presetNameOpt.multiline = false;
	presetNameOpt.transform = [=](InputState state) {
		auto e = state.element | align_right;
		if (state.is_placeholder)
			return e | color(toggleOffColor);
		return e | color(toggleOnColor);
	};
	auto presetNameInput =
		Input(&m_editingPresetName, "preset name", presetNameOpt);

	auto presetBtnStyle = ButtonOption::Animated();
	presetBtnStyle.transform = [=](const EntryState &s) {
		auto e = text(s.label) | color(Color::MagentaLight);
		if (s.focused)
			e |= bold;
		return e | center;
	};

	auto presetSaveBtn = Button(
		"Save",
		[this] { saveCurrentToPreset(); },
		presetBtnStyle);

	auto presetRenameBtn = Button(
		"Rename",
		[this] { renameSelectedPreset(m_editingPresetName); },
		presetBtnStyle);

	auto presetNewBtn = Button(
		"+ New",
		[this] {
			m_selectedPresetIndex = -1;
			std::string base =
				m_selectedModelName.empty() ? "preset" : m_selectedModelName;
			m_editingPresetName = base + "-new";
			m_presetStatus.clear();
		},
		presetBtnStyle);

	auto presetDeleteBtn = Button(
		"Delete",
		[this] {
			if (m_selectedPresetIndex >= 0 &&
				m_selectedPresetIndex <
					static_cast<int>(m_presetsForModel.size())) {
				std::string name = m_presetsForModel[m_selectedPresetIndex].name;
				if (ModelsIni::instance().deletePreset(name)) {
					m_presetStatus = "Deleted";
					m_selectedPresetIndex = -1;
					m_editingPresetName.clear();
					refreshPresetsForModel();
				}
			}
		},
		presetBtnStyle);

	// -----------------------------------------------------------------------
	// Model Selection Dropdown, START/STOP, and LOAD/UNLOAD Buttons
	// -----------------------------------------------------------------------
	// Model dropdown - selecting a model updates m_modelPath which is used when
	// clicking LOAD/UNLOAD Note: Using Dropdown component for scrollable list
	// (works with 100+ models)
	auto modelDropdown = Dropdown(&m_modelDisplayNames, &m_modelDropdownIndex);

	// Single button that changes based on server state:
	// - Server not running: shows "LOAD" (green) - starts the server
	// - Server running, no model: shows "LOAD" (green) - loads selected model
	// via API
	// - Server running, model loaded: shows "UNLOAD" (red) - unloads current
	// model
	auto serverButton = Button(
		&m_startStopLabel,
		[this] {
			auto &process = LlamaServerProcess::instance();
			if (!process.isRunning()) {
				// Server not running - START it
				onStartStopClicked();
			} else {
				// Server is running - LOAD or UNLOAD model
				onLoadUnloadClicked();
			}
		},
		startStopBtnStyle);

	// -----------------------------------------------------------------------
	// Inference components
	// Creates controls for text generation parameters (temperature, sampling,
	// penalties) Float controls use 0.01 step for fine-grained probability
	// adjustments
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
	auto seedInput = Input(&m_seed, "-1 = random", inputOpt);

	// -----------------------------------------------------------------------
	// Container — two-column layout: Load Settings (left), Inference (right)
	// Uses FTXUI's Container::Horizontal to split the panel vertically
	// Each column is a vertical stack of interactive components
	// The Renderer below wraps each column in a labeled window
	// -----------------------------------------------------------------------
	auto container = Container::Horizontal({
		Container::Vertical({
			// Left column: Load Settings
			gpuLayersInput,
			devicePriorityInput,
			ctxSizeInput,
			batchSizeMinus,
			batchSizeInput,
			batchSizePlus,
			parallelMinus,
			parallelInput,
			parallelPlus,
			flashAttnToggle,
			splitModeToggle,
			tensorSplitInput,
			cacheTypeKToggle,
			cacheTypeVToggle,
			loraInput,
			mmprojInput,
			modelDraftInput,
			draftMaxInput,
			chatTemplateInput,
			reasoningFormatToggle,
			kvOffloadCb,
			kvUnifiedCb,
			mmapCb,
			mlockCb,
			fitCb,
			// Presets
			presetMenu,
			presetNameInput,
			presetSaveBtn,
			presetRenameBtn,
			presetNewBtn,
			presetDeleteBtn,
			// Footer: model dropdown and single server button
			modelDropdown,
			serverButton,
		}),
		Container::Vertical({
			// Right column: Inference Settings
			tempMinus,		tempInput,		tempPlus,	   topPMinus,
			topPInput,		topPPlus,		topKMinus,	   topKInput,
			topKPlus,		minPMinus,		minPInput,	   minPPlus,
			repeatPenMinus, repeatPenInput, repeatPenPlus, presPenMinus,
			presPenInput,	presPenPlus,	freqPenMinus,  freqPenInput,
			freqPenPlus,	nPredictInput,	seedInput,
		}),
	});

	// Track model dropdown changes to refresh presets
	int lastModelIndex = m_modelDropdownIndex;
	// Periodic refresh counter - refresh every ~2 seconds (60 renders at 30Hz)
	int renderCount = 0;
	auto lastRefreshTime = std::chrono::steady_clock::now();

	m_component = Renderer(container, [=, this]() mutable {
		// Periodic server state refresh to keep button state accurate
		renderCount++;
		auto now = std::chrono::steady_clock::now();
		auto elapsed =
			std::chrono::duration_cast<std::chrono::seconds>(now - lastRefreshTime)
				.count();
		if (elapsed >= 2) { // Refresh every 2 seconds
			lastRefreshTime = now;
			refreshServerState();
			updateStartStopLabel();
		}

		// Detect model dropdown change
		if (m_modelDropdownIndex != lastModelIndex) {
			lastModelIndex = m_modelDropdownIndex;
			if (m_modelDropdownIndex >= 0 &&
				m_modelDropdownIndex < static_cast<int>(m_modelNames.size())) {
				m_selectedModelName = m_modelNames[m_modelDropdownIndex];
				m_modelPath = m_modelPaths[m_modelDropdownIndex];
			}
			refreshPresetsForModel();

			// Auto-select first preset if any exist
			if (!m_presetsForModel.empty()) {
				m_selectedPresetIndex = 0;
				applyPreset(m_presetsForModel[0]);
				m_editingPresetName = m_presetsForModel[0].name;
			} else {
				m_selectedPresetIndex = -1;
				m_editingPresetName.clear();
			}
			m_presetStatus.clear();
			updateStartStopLabel();
			saveConfig();
		}

		// === Left column: Load Settings ===
		Elements leftElements;
		{
			Elements rows;
			rows.push_back(
				ui_utils::settingRowComponent("GPU Layers",
											  gpuLayersInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Device Priority",
											  devicePriorityInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Context Size",
											  ctxSizeInput->Render()));
			rows.push_back(ui_utils::numberRow("Batch Size",
											   batchSizeMinus->Render(),
											   batchSizeInput->Render(),
											   batchSizePlus->Render()));
			rows.push_back(ui_utils::numberRow("Parallel Slots",
											   parallelMinus->Render(),
											   parallelInput->Render(),
											   parallelPlus->Render()));
			rows.push_back(ui_utils::checkboxRow("Flash Attention",
												 flashAttnToggle->Render()));
			rows.push_back(
				ui_utils::checkboxRow("Split Mode", splitModeToggle->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Tensor Split",
											  tensorSplitInput->Render()));
			rows.push_back(ui_utils::checkboxRow("Cache Type K",
												 cacheTypeKToggle->Render()));
			rows.push_back(ui_utils::checkboxRow("Cache Type V",
												 cacheTypeVToggle->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("LoRA", loraInput->Render()));
			rows.push_back(ui_utils::settingRowComponent("MM Projector",
														 mmprojInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Draft Model",
											  modelDraftInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Draft Max",
											  draftMaxInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Chat Template",
											  chatTemplateInput->Render()));
			rows.push_back(
				ui_utils::checkboxRow("Reasoning Format",
									  reasoningFormatToggle->Render()));
			rows.push_back(ui_utils::checkboxRow("KV Cache Offload",
												 kvOffloadCb->Render()));
			rows.push_back(
				ui_utils::checkboxRow("KV Unified", kvUnifiedCb->Render()));
			rows.push_back(
				ui_utils::checkboxRow("Memory Map", mmapCb->Render()));
			rows.push_back(
				ui_utils::checkboxRow("Memory Lock", mlockCb->Render()));
			rows.push_back(
				ui_utils::checkboxRow("Fit to Memory", fitCb->Render()));
			leftElements.push_back(
				window(text("Load Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		// === Right column: Inference Settings ===
		Elements rightElements;
		{
			Elements rows;
			rows.push_back(ui_utils::numberRow("Temperature",
											   tempMinus->Render(),
											   tempInput->Render(),
											   tempPlus->Render()));
			rows.push_back(ui_utils::numberRow("Top P",
											   topPMinus->Render(),
											   topPInput->Render(),
											   topPPlus->Render()));
			rows.push_back(ui_utils::numberRow("Top K",
											   topKMinus->Render(),
											   topKInput->Render(),
											   topKPlus->Render()));
			rows.push_back(ui_utils::numberRow("Min P",
											   minPMinus->Render(),
											   minPInput->Render(),
											   minPPlus->Render()));
			rows.push_back(ui_utils::numberRow("Repeat Penalty",
											   repeatPenMinus->Render(),
											   repeatPenInput->Render(),
											   repeatPenPlus->Render()));
			rows.push_back(ui_utils::numberRow("Presence Penalty",
											   presPenMinus->Render(),
											   presPenInput->Render(),
											   presPenPlus->Render()));
			rows.push_back(ui_utils::numberRow("Frequency Penalty",
											   freqPenMinus->Render(),
											   freqPenInput->Render(),
											   freqPenPlus->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Max Tokens",
											  nPredictInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Seed", seedInput->Render()));
			rightElements.push_back(
				window(text("Inference Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		auto leftCol = vbox(std::move(leftElements)) | flex;
		auto rightCol = vbox(std::move(rightElements)) | flex;

		// === Presets panel ===
		Elements presetElements;
		if (m_presetDisplayNames.empty()) {
			presetElements.push_back(
				text("  No presets — use Save to create one.") |
				color(Color::GrayDark));
		} else {
			presetElements.push_back(presetMenu->Render() | vscroll_indicator |
									 frame | size(HEIGHT, LESS_THAN, 6));
		}
		// Name input + buttons row
		presetElements.push_back(separatorLight());
		presetElements.push_back(hbox({
			text(" Name: ") | color(Color::GrayLight),
			presetNameInput->Render() | flex,
			separatorLight(),
			presetRenameBtn->Render(),
			separatorLight(),
			presetSaveBtn->Render(),
			separatorLight(),
			presetNewBtn->Render(),
			separatorLight(),
			presetDeleteBtn->Render(),
		}));
		// Status message
		if (!m_presetStatus.empty()) {
			Color statusColor = (m_presetStatus == "Name in use" ||
								 m_presetStatus == "Save failed" ||
								 m_presetStatus == "Rename failed")
									? Color::RedLight
									: Color::GreenLight;
			presetElements.push_back(text("  " + m_presetStatus) |
									 color(statusColor));
		}

		auto presetsPanel = window(text("Presets") | bold | color(Color::Yellow),
								   vbox(std::move(presetElements)),
								   ftxui::EMPTY);

		// Footer section with model dropdown and single server button
		auto footerRow = hbox({
			filler(),
			vbox({
				modelDropdown->Render(),
				separatorLight(),
				serverButton->Render(),
			}) | flex,
			filler(),
		});

		return vbox({
				   hbox({ leftCol, separatorLight(), rightCol }),
				   presetsPanel,
				   filler(),
				   footerRow,
			   }) |
			   xflex | yflex;
	});

	return m_component;
}

// =========================================================================
// Preset Methods
// =========================================================================

void ModelsPanel::refreshPresetsForModel()
{
	if (m_selectedModelName.empty()) {
		m_presetsForModel.clear();
		m_presetDisplayNames.clear();
		return;
	}
	// Find the model path from our parallel vector
	std::string modelPath;
	for (size_t i = 0; i < m_modelNames.size(); ++i) {
		if (m_modelNames[i] == m_selectedModelName) {
			modelPath = m_modelPaths[i];
			break;
		}
	}
	if (modelPath.empty())
		modelPath = ModelsIni::instance().getModelPath(m_selectedModelName);
	m_presetsForModel = ModelsIni::instance().getPresetsForModel(modelPath);
	m_presetDisplayNames.clear();
	for (const auto &p : m_presetsForModel)
		m_presetDisplayNames.push_back(p.name);

	// Clamp selected index
	if (m_selectedPresetIndex >= static_cast<int>(m_presetsForModel.size()))
		m_selectedPresetIndex = -1;
}

void ModelsPanel::applyPreset(const Config::ModelPreset &preset)
{
	// Load settings
	m_ngpuLayers = preset.load.ngpuLayers;
	m_ctxSize =
		preset.load.ctxSize == 0 ? "" : std::to_string(preset.load.ctxSize);
	m_batchSize = preset.load.batchSize;
	m_batchSizeStr = std::to_string(m_batchSize);
	m_parallel = preset.load.parallel;
	m_parallelStr = m_parallel < 0 ? "-1" : std::to_string(m_parallel);

	// Flash attention dropdown index
	if (preset.load.flashAttn == "on")
		m_flashAttnIdx = 1;
	else if (preset.load.flashAttn == "off")
		m_flashAttnIdx = 2;
	else
		m_flashAttnIdx = 0;

	m_kvOffload = preset.load.kvOffload;
	m_kvUnified = preset.load.kvUnified;
	m_mmap = preset.load.mmap;
	m_mlock = preset.load.mlock;
	m_fit = preset.load.fit;
	m_devicePriority = preset.load.devicePriority;
	m_splitMode = preset.load.splitMode;
	m_tensorSplit = preset.load.tensorSplit;
	m_cacheTypeK = preset.load.cacheTypeK;
	m_cacheTypeV = preset.load.cacheTypeV;
	m_lora = preset.load.lora;
	m_mmproj = preset.load.mmproj;
	m_modelDraft = preset.load.modelDraft;
	m_draftMax = std::to_string(preset.load.draftMax);
	m_chatTemplate = preset.load.chatTemplate;
	m_reasoningFormat = preset.load.reasoningFormat;

	// Dropdown indices for load settings
	m_splitModeIdx = findOptionIndex(m_splitModeOptions, preset.load.splitMode);
	m_cacheTypeKIdx =
		findOptionIndex(m_cacheTypeOptions, preset.load.cacheTypeK);
	m_cacheTypeVIdx =
		findOptionIndex(m_cacheTypeOptions, preset.load.cacheTypeV);
	m_reasoningFormatIdx =
		findOptionIndex(m_reasoningFormatOptions, preset.load.reasoningFormat);

	// Inference settings
	m_temperature = static_cast<float>(preset.inference.temperature);
	m_temperatureStr = ui_utils::formatFloat(m_temperature);
	m_topP = static_cast<float>(preset.inference.topP);
	m_topPStr = ui_utils::formatFloat(m_topP);
	m_topK = preset.inference.topK;
	m_topKStr = std::to_string(m_topK);
	m_minP = static_cast<float>(preset.inference.minP);
	m_minPStr = ui_utils::formatFloat(m_minP);
	m_repeatPenalty = static_cast<float>(preset.inference.repeatPenalty);
	m_repeatPenaltyStr = ui_utils::formatFloat(m_repeatPenalty);
	m_presencePenalty = static_cast<float>(preset.inference.presencePenalty);
	m_presencePenaltyStr = ui_utils::formatFloat(m_presencePenalty);
	m_frequencyPenalty = static_cast<float>(preset.inference.frequencyPenalty);
	m_frequencyPenaltyStr = ui_utils::formatFloat(m_frequencyPenalty);
	m_nPredict = std::to_string(preset.inference.nPredict);
	m_seed = std::to_string(preset.inference.seed);

	// Persist to config.json so LOAD picks up the values
	saveConfig();

	spdlog::info("Applied preset '{}'", preset.name);
}

void ModelsPanel::saveCurrentToPreset()
{
	Config::ModelPreset preset;

	// Determine preset name
	if (m_editingPresetName.empty()) {
		// Generate default name
		std::string base =
			m_selectedModelName.empty() ? "preset" : m_selectedModelName;
		preset.name = base + "-1";
		int n = 1;
		// Ensure unique
		while (true) {
			bool found = false;
			for (const auto &p : m_presetsForModel) {
				if (p.name == preset.name) {
					found = true;
					break;
				}
			}
			if (!found)
				break;
			preset.name = base + "-" + std::to_string(++n);
		}
		m_editingPresetName = preset.name;
	} else {
		preset.name = m_editingPresetName;
	}

	// Use model path from our parallel vector
	for (size_t i = 0; i < m_modelNames.size(); ++i) {
		if (m_modelNames[i] == m_selectedModelName) {
			preset.model = m_modelPaths[i];
			break;
		}
	}
	if (preset.model.empty())
		preset.model = m_modelPath; // fallback to current model path

	// Load settings from current member state
	preset.load.modelPath = preset.model;
	preset.load.ngpuLayers = m_ngpuLayers;
	try {
		preset.load.ctxSize = m_ctxSize.empty() ? 0 : std::stoi(m_ctxSize);
	} catch (...) {
	}
	preset.load.batchSize = m_batchSize;
	preset.load.parallel = m_parallel;
	preset.load.flashAttn =
		m_flashAttnOptions[static_cast<size_t>(m_flashAttnIdx)];
	preset.load.kvOffload = m_kvOffload;
	preset.load.kvUnified = m_kvUnified;
	preset.load.mmap = m_mmap;
	preset.load.mlock = m_mlock;
	preset.load.fit = m_fit;
	preset.load.devicePriority = m_devicePriority;
	preset.load.splitMode = m_splitMode;
	preset.load.tensorSplit = m_tensorSplit;
	preset.load.cacheTypeK =
		m_cacheTypeOptions[static_cast<size_t>(m_cacheTypeKIdx)];
	preset.load.cacheTypeV =
		m_cacheTypeOptions[static_cast<size_t>(m_cacheTypeVIdx)];
	preset.load.lora = m_lora;
	preset.load.mmproj = m_mmproj;
	preset.load.modelDraft = m_modelDraft;
	try {
		preset.load.draftMax = std::stoi(m_draftMax);
	} catch (...) {
	}
	preset.load.chatTemplate = m_chatTemplate;
	preset.load.reasoningFormat =
		m_reasoningFormatOptions[static_cast<size_t>(m_reasoningFormatIdx)];

	// Inference settings
	preset.inference.temperature = static_cast<double>(m_temperature);
	preset.inference.topP = static_cast<double>(m_topP);
	preset.inference.topK = m_topK;
	preset.inference.minP = static_cast<double>(m_minP);
	preset.inference.repeatPenalty = static_cast<double>(m_repeatPenalty);
	preset.inference.presencePenalty = static_cast<double>(m_presencePenalty);
	preset.inference.frequencyPenalty = static_cast<double>(m_frequencyPenalty);
	try {
		preset.inference.nPredict = std::stoi(m_nPredict);
	} catch (...) {
	}
	try {
		preset.inference.seed = std::stoi(m_seed);
	} catch (...) {
	}

	if (ModelsIni::instance().savePreset(preset)) {
		m_presetStatus = "Saved";
		refreshPresetsForModel();
		// Select the saved preset
		for (int i = 0; i < static_cast<int>(m_presetsForModel.size()); ++i) {
			if (m_presetsForModel[i].name == preset.name) {
				m_selectedPresetIndex = i;
				break;
			}
		}
	} else {
		m_presetStatus = "Save failed";
	}
}

void ModelsPanel::renameSelectedPreset(const std::string &newName)
{
	if (m_selectedPresetIndex < 0 ||
		m_selectedPresetIndex >= static_cast<int>(m_presetsForModel.size())) {
		return;
	}

	std::string oldName = m_presetsForModel[m_selectedPresetIndex].name;
	if (oldName == newName || newName.empty())
		return;

	// Check for duplicate name
	for (const auto &p : m_presetsForModel) {
		if (p.name == newName) {
			m_presetStatus = "Name in use";
			return;
		}
	}

	if (ModelsIni::instance().renamePreset(oldName, newName)) {
		m_presetStatus = "Renamed";
		refreshPresetsForModel();
		// Re-select by new name
		for (int i = 0; i < static_cast<int>(m_presetsForModel.size()); ++i) {
			if (m_presetsForModel[i].name == newName) {
				m_selectedPresetIndex = i;
				break;
			}
		}
	} else {
		m_presetStatus = "Rename failed";
	}
}

// =========================================================================
// Server Methods
// =========================================================================

void ModelsPanel::onStartStopClicked()
{
	if (LlamaServerProcess::instance().isRunning()) {
		// Stop: terminate process
		LlamaServerProcess::instance().terminate();
		m_serverRunning = false;
		m_modelLoaded = false;
		m_loadedModelPath.clear();
		m_startStopLabel = "LOAD"; // Ready to start again
		spdlog::info("Server stopped");
	} else {
		// Start: launch server (without model initially)
		auto &cfg = ConfigManager::instance().getConfig();

		// Launch with empty model path = server only, no model
		bool success = LlamaServerProcess::instance().launch(
			"", // Empty model path = no model
			cfg.load,
			cfg.inference,
			cfg.server);

		if (success) {
			m_serverStarting.store(true);
			m_startStopLabel = "STARTING...";
			std::thread([this] {
				for (int i = 0; i < 20; ++i) {
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					if (LlamaServerProcess::instance().isServerHealthy()) {
						break;
					}
				}
				m_serverStarting.store(false);
				updateStartStopLabel();
				spdlog::info("Server started (no model)");
			}).detach();
		} else {
			spdlog::error("Failed to start server");
		}
	}
}

void ModelsPanel::onLoadUnloadClicked()
{
	auto &process = LlamaServerProcess::instance();

	// Validate model selection
	if (m_modelNames.empty() || m_modelDropdownIndex < 0 ||
		m_modelDropdownIndex >= static_cast<int>(m_modelNames.size())) {
		spdlog::warn("No model selected, cannot load");
		return;
	}

	// Use section name from models.ini for the API call
	std::string selectedModel = m_modelNames[m_modelDropdownIndex];

	// Button label determines action, not m_modelLoaded state
	if (m_startStopLabel == "UNLOAD") {
		// Unload: call unloadModel API (unload whatever is currently loaded)
		(void)process.unloadModel();
		// ALWAYS verify actual state after unload attempt
		refreshServerState();
		updateStartStopLabel();
		if (!m_modelLoaded) {
			spdlog::info("Model unloaded");
		}
	} else {
		// LOAD: load the selected model (switches if different model is loaded)
		if (!process.isRunning()) {
			// Auto-start server if not running
			onStartStopClicked();
			// Server start is async — the health poll thread will handle it
			return;
		}

		// Load the model via API - pass section name, not path
		(void)process.loadModel(selectedModel);
		// ALWAYS verify actual state after load attempt
		refreshServerState();
		updateStartStopLabel();
		if (m_modelLoaded) {
			spdlog::info("Model loaded: {}", selectedModel);
		} else {
			spdlog::error("Failed to load model: {}", selectedModel);
		}
	}
}

void ModelsPanel::refreshServerState()
{
	auto &process = LlamaServerProcess::instance();
	m_serverRunning = process.isRunning() && process.isServerHealthy();
	m_modelLoaded = m_serverRunning && process.isModelLoaded();
	if (m_modelLoaded) {
		m_loadedModelPath = process.getLoadedModelPath();
	} else {
		m_loadedModelPath.clear();
	}
}

void ModelsPanel::updateStartStopLabel()
{
	// Get the currently selected model in dropdown
	std::string selectedModel;
	if (m_modelDropdownIndex >= 0 &&
		m_modelDropdownIndex < static_cast<int>(m_modelNames.size())) {
		selectedModel = m_modelNames[m_modelDropdownIndex];
	}

	if (!LlamaServerProcess::instance().isRunning()) {
		// Server not running - show LOAD
		m_startStopLabel = "LOAD";
	} else if (!m_modelLoaded) {
		// Server running but no model loaded - show LOAD
		m_startStopLabel = "LOAD";
	} else {
		// Server running with a model loaded
		// Show UNLOAD if the selected model matches what we loaded,
		// otherwise show LOAD so the user can switch to the selected model
		if (m_loadedModelPath == selectedModel) {
			m_startStopLabel = "UNLOAD";
		} else {
			m_startStopLabel = "LOAD";
		}
	}
}
