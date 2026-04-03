#include "modelsPanel.h"
#include "configManager.h"
#include "modelDiscovery.h"
#include "ui_utils.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <iostream>

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

using namespace ftxui;

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

	// Load settings
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

	m_kvOffload = cfg.load.kvOffload;
	m_mmap = cfg.load.mmap;
	m_mlock = cfg.load.mlock;
	m_fit = cfg.load.fit;
	m_devicePriority = cfg.load.devicePriority;

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

	// Phase 1: Refresh model list and try to select config.load.modelPath
	refreshModelList();

	// Try to select the model matching config.load.modelPath (if found)
	for (size_t i = 0; i < m_modelPaths.size(); ++i) {
		if (m_modelPaths[i] == cfg.load.modelPath) {
			m_modelDropdownIndex = static_cast<int>(i);
			m_selectedModelPath = m_modelPaths[i];
			break;
		}
	}
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
	cfg.load.flashAttn = m_flashAttnOptions[static_cast<size_t>(m_flashAttnIdx)];
	cfg.load.kvOffload = m_kvOffload;
	cfg.load.mmap = m_mmap;
	cfg.load.mlock = m_mlock;
	cfg.load.fit = m_fit;
	cfg.load.devicePriority = m_devicePriority;

	// Inference settings
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
}

// =========================================================================
// Phase 1: Model Discovery Integration
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
	// Scan for models using ModelDiscovery singleton
	auto models = ModelDiscovery::instance().scanForModels();

	m_modelPaths.clear();
	m_modelDisplayNames.clear();

	for (const auto &path : models) {
		// Phase 3: Apply file filter - skip models matching filter patterns
		if (shouldFilterModel(path)) {
			continue;
		}

		m_modelPaths.push_back(path);
		m_modelDisplayNames.push_back(
			ModelDiscovery::instance().pathToDisplayName(path));
	}

	// Sort by display name for user-friendly dropdown
	// Keep paths and names in sync
	std::vector<size_t> indices(m_modelPaths.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
		return m_modelDisplayNames[a] < m_modelDisplayNames[b];
	});

	std::vector<std::string> sortedPaths, sortedNames;
	for (auto idx : indices) {
		sortedPaths.push_back(m_modelPaths[idx]);
		sortedNames.push_back(m_modelDisplayNames[idx]);
	}
	m_modelPaths = std::move(sortedPaths);
	m_modelDisplayNames = std::move(sortedNames);

	// Reset dropdown index if it's now out of bounds
	if (m_modelDropdownIndex < 0 ||
		m_modelDropdownIndex >= static_cast<int>(m_modelPaths.size())) {
		m_modelDropdownIndex = 0;
		if (!m_modelPaths.empty()) {
			m_selectedModelPath = m_modelPaths[0];
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

	auto flashAttnOpt = toggleOpt;
	flashAttnOpt.entries = &m_flashAttnOptions;
	flashAttnOpt.selected = &m_flashAttnIdx;
	auto flashAttnToggle = Menu(flashAttnOpt);
	auto kvOffloadCb = Checkbox("", &m_kvOffload, cbOpt);
	auto mmapCb = Checkbox("", &m_mmap, cbOpt);
	auto mlockCb = Checkbox("", &m_mlock, cbOpt);
	auto fitCb = Checkbox("", &m_fit, cbOpt);

	// -----------------------------------------------------------------------
	// Phase 1: Model Selection Dropdown and LOAD Button
	// -----------------------------------------------------------------------
	// Model dropdown - selecting a model updates m_modelPath which is used when
	// clicking LOAD Note: Using Dropdown component for scrollable list (works
	// with 100+ models)
	auto modelDropdown = Dropdown(&m_modelDisplayNames, &m_modelDropdownIndex);

	// Custom on_change handler for dropdown - using a separate Button to trigger
	// the sync Since Dropdown doesn't have on_change, we update m_modelPath in
	// onLoadClicked() But we need to make sure it's updated BEFORE launch

	// LOAD button - launches llama-server with selected model
	auto loadButton = Button(
		"LOAD",
		[this] { onLoadClicked(); },
		loadBtnStyle);

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
			flashAttnToggle,
			kvOffloadCb,
			mmapCb,
			mlockCb,
			fitCb,
			// Footer: model dropdown and load button
			modelDropdown,
			loadButton,
		}),
		Container::Vertical({
			// Right column: Inference Settings
			tempMinus,		tempInput,		tempPlus,	   topPMinus,
			topPInput,		topPPlus,		topKMinus,	   topKInput,
			topKPlus,		minPMinus,		minPInput,	   minPPlus,
			repeatPenMinus, repeatPenInput, repeatPenPlus, presPenMinus,
			presPenInput,	presPenPlus,	freqPenMinus,  freqPenInput,
			freqPenPlus,	nPredictInput,
		}),
	});

	m_component = Renderer(container, [=, this] {
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
			rows.push_back(ui_utils::checkboxRow("Flash Attention",
												 flashAttnToggle->Render()));
			rows.push_back(ui_utils::checkboxRow("KV Cache Offload",
												 kvOffloadCb->Render()));
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
			rightElements.push_back(
				window(text("Inference Settings") | bold | color(Color::Yellow),
					   hbox({ text("    "), vbox(std::move(rows)) | xflex }),
					   ftxui::EMPTY));
		}

		auto leftCol = vbox(std::move(leftElements)) | flex;
		auto rightCol = vbox(std::move(rightElements)) | flex;

		// Footer section with model dropdown above LOAD button
		auto footerRow = hbox({
			filler(),
			vbox({
				modelDropdown->Render(),
				separatorLight(),
				loadButton->Render() | bgcolor(Color::MagentaLight),
			}),
			filler(),
		});

		return vbox({
				   hbox({ leftCol, separatorLight(), rightCol }),
				   filler(),
				   footerRow,
			   }) |
			   xflex | yflex;
	});

	return m_component;
}

void ModelsPanel::onLoadClicked()
{
	// Validate model selection
	if (m_modelPaths.empty() || m_modelDropdownIndex < 0 ||
		m_modelDropdownIndex >= static_cast<int>(m_modelPaths.size())) {
		return;
	}

	// Get selected model path
	std::string modelPath = m_modelPaths[m_modelDropdownIndex];

	// Get current config
	auto &cfg = ConfigManager::instance().getConfig();

	// If already running, terminate first
	if (LlamaServerProcess::instance().isRunning()) {
		LlamaServerProcess::instance().terminate();
	}

	// Launch the server using singleton for global access during cleanup
	bool success = LlamaServerProcess::instance().launch(modelPath,
														 cfg.load,
														 cfg.inference,
														 cfg.server);
}
