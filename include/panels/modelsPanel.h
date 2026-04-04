#pragma once

#include "configManager.h"
#include "llamaServerProcess.h"
#include "modelDiscovery.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>

/**
 * @file modelsPanel.h
 * @brief Panel containing Load and Inference configuration settings.
 *
 * This panel provides interactive controls for configuring model loading
 * parameters and inference behavior. It is a stateful component that
 * reads from and writes to the ConfigManager on change.
 *
 * Sections:
 * - Load Settings: Model path, GPU layers, context size, batch size,
 *   flash attention, memory mapping options
 * - Inference Settings: Temperature, top-P, top-K, min-P, penalties,
 *   and max tokens prediction
 */
class ModelsPanel
{
  public:
	/**
	 * @brief Constructs the ModelsPanel and loads current configuration.
	 *
	 * Initializes all member variables from ConfigManager::getConfig().
	 */
	ModelsPanel();

	/**
	 * @brief Returns the FTXUI component for this panel.
	 *
	 * The component is cached after first creation and reused on
	 * subsequent calls.
	 *
	 * @return An @c ftxui::Component containing the Load and Inference
	 *         settings controls.
	 */
	ftxui::Component component();

  private:
	/**
	 * @brief Loads current configuration values into member variables.
	 *
	 * Called from constructor to initialize state. Reads Load and
	 * Inference sections from ConfigManager.
	 */
	void loadFromConfig();

	/**
	 * @brief Saves current member variable values to configuration.
	 *
	 * Called whenever a setting is changed (via onChange callbacks).
	 * Writes Load and Inference sections to ConfigManager and persists
	 * to disk.
	 */
	void saveConfig();

	// =========================================================================
	// Member component
	// =========================================================================
	ftxui::Component m_component;

	// =========================================================================
	// Load Settings State
	// =========================================================================
	std::string m_modelPath;
	std::string m_ngpuLayers;
	std::string m_ctxSize;
	int m_batchSize = 2048;
	std::string m_batchSizeStr = "2048";
	int m_parallel = 4; // -1=auto, or number of slots
	std::string m_parallelStr = "4";
	int m_flashAttnIdx = 0; // 0=auto, 1=on, 2=off
	bool m_kvOffload = true;
	bool m_kvUnified = true;
	bool m_mmap = false;
	bool m_mlock = false;
	bool m_fit = true;
	std::string m_devicePriority = ""; // ""=auto, "0"=GPU0 first, "1"=GPU1 first
	std::string m_splitMode;		   // ""=none, "layer", "row"
	std::string m_tensorSplit;		   // comma-separated ratios
	std::string m_cacheTypeK = "f16";
	std::string m_cacheTypeV = "f16";
	std::string m_lora;
	std::string m_mmproj;
	std::string m_modelDraft;
	std::string m_draftMax = "-1";
	std::string m_chatTemplate;
	std::string m_reasoningFormat;

	// =========================================================================
	// Inference Settings State
	// =========================================================================
	float m_temperature = 0.8f;
	std::string m_temperatureStr;
	float m_topP = 0.95f;
	std::string m_topPStr;
	int m_topK = 40;
	std::string m_topKStr = "40";
	float m_minP = 0.05f;
	std::string m_minPStr;
	float m_repeatPenalty = 1.0f;
	std::string m_repeatPenaltyStr;
	float m_presencePenalty = 0.0f;
	std::string m_presencePenaltyStr;
	float m_frequencyPenalty = 0.0f;
	std::string m_frequencyPenaltyStr;
	std::string m_nPredict;
	std::string m_seed = "-1"; // -1 = random

	// =========================================================================
	// Dropdown Options
	// =========================================================================
	std::vector<std::string> m_flashAttnOptions = { "auto", "on", "off" };
	std::vector<std::string> m_splitModeOptions = { "none", "layer", "row" };
	std::vector<std::string> m_cacheTypeOptions = { "f16",	  "f32",  "bf16",
													"q8_0",	  "q4_0", "q4_1",
													"iq4_nl", "q5_0", "q5_1" };
	std::vector<std::string> m_reasoningFormatOptions = {
		"auto", "default", "none", "hidden", "deepseek", "deepseek-legacy"
	};
	int m_splitModeIdx = 1;		  // 1 = "layer" (default)
	int m_cacheTypeKIdx = 0;	  // 0 = "f16" (default)
	int m_cacheTypeVIdx = 0;	  // 0 = "f16" (default)
	int m_reasoningFormatIdx = 0; // 0 = "auto" (default)

	// =========================================================================
	// Model Discovery & Selection (Phase 1)
	// =========================================================================
	std::vector<std::string> m_modelPaths;		  // Full paths from discovery
	std::vector<std::string> m_modelDisplayNames; // Display names for dropdown
	int m_modelDropdownIndex = 0;				  // Selected index in dropdown
	std::string m_selectedModelPath;			  // Full path of selected model

	/** Refresh model list from ModelDiscovery singleton. */
	void refreshModelList();

	/**
	 * @brief Check if a model path should be filtered out based on fileFilter
	 * patterns.
	 *
	 * Implements glob-style wildcard matching (case-insensitive).
	 *
	 * @param path Full or partial path to the model file
	 * @return true if the model should be filtered out (excluded), false
	 * otherwise
	 */
	bool shouldFilterModel(const std::string &path) const;

	// =========================================================================
	// Llama Server Process (Phase 2)
	// =========================================================================
	/** Handle LOAD button click - launch llama-server with selected model. */
	void onLoadClicked();
};
