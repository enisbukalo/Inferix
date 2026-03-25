#include "modelsPanel.h"
#include "configManager.h"
#include "ui_utils.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include <algorithm>
#include <iomanip>
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

	m_mmap = cfg.load.mmap;
	m_mlock = cfg.load.mlock;
	m_fit = cfg.load.fit;

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
	cfg.load.mmap = m_mmap;
	cfg.load.mlock = m_mlock;
	cfg.load.fit = m_fit;

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
			modelPathInput,
			gpuLayersInput,
			ctxSizeInput,
			batchSizeMinus,
			batchSizeInput,
			batchSizePlus,
			flashAttnToggle,
			mmapCb,
			mlockCb,
			fitCb,
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
				ui_utils::settingRowComponent("Model Path",
											  modelPathInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("GPU Layers",
											  gpuLayersInput->Render()));
			rows.push_back(
				ui_utils::settingRowComponent("Context Size",
											  ctxSizeInput->Render()));
			rows.push_back(ui_utils::numberRow("Batch Size",
											   batchSizeMinus->Render(),
											   batchSizeInput->Render(),
											   batchSizePlus->Render()));
			rows.push_back(ui_utils::checkboxRow("Flash Attention",
												 flashAttnToggle->Render()));
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

		return hbox({ leftCol, separatorLight(), rightCol });
	});

	return m_component;
}
