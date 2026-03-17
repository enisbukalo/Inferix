/**
 * @file settingsPanel.cpp
 * @brief Settings panel implementation.
 *
 * Implements a panel that displays all application configuration settings
 * organized by category. Uses hbox/vbox layout instead of tables to allow
 * future replacement of text values with interactive widgets.
 */

#include "settingsPanel.h"
#include "configManager.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

/**
 * @brief Creates a setting row with label and value.
 *
 * @param label The setting name
 * @param value The setting value as a string
 * @return An hbox element with the label and value
 */
Element settingRow(const std::string &label, const std::string &value)
{
	return hbox({
		text("        "),
		text(label) | color(Color::MagentaLight),
		filler(),
		text(value) | color(Color::CyanLight),
	});
}

/**
 * @brief Creates a category header.
 *
 * @param title The category title
 * @return An hbox element with the category header
 */
Element categoryHeader(const std::string &title)
{
	return hbox({
			   text("--") | bold | color(Color::YellowLight),
			   text(title) | bold | color(Color::YellowLight),
			   text("--") | bold | color(Color::YellowLight),
		   }) |
		   flex;
}

/**
 * @brief Converts a boolean to "Enabled" or "Disabled" string.
 *
 * @param value The boolean value
 * @return "Enabled" if true, "Disabled" if false
 */
std::string boolToString(bool value)
{
	return value ? "Enabled" : "Disabled";
}

Element SettingsPanel::render()
{
	auto config = ConfigManager::instance().getConfig();

	Elements settings;

	// Server Settings
	settings.push_back(categoryHeader("Server Settings"));
	settings.push_back(settingRow("Host", config.server.host));
	settings.push_back(settingRow("Port", std::to_string(config.server.port)));
	settings.push_back(
		settingRow("API Key",
				   config.server.apiKey.empty() ? "[NOT SET]" : "[SET]"));
	settings.push_back(
		settingRow("Timeout", std::to_string(config.server.timeout) + "s"));
	settings.push_back(
		settingRow("HTTP Threads",
				   config.server.threadsHttp == -1
					   ? "auto"
					   : std::to_string(config.server.threadsHttp)));
	settings.push_back(settingRow("Web UI", boolToString(config.server.webui)));
	settings.push_back(
		settingRow("Embedding Mode", boolToString(config.server.embedding)));
	settings.push_back(settingRow("Continuous Batching",
								  boolToString(config.server.contBatching)));
	settings.push_back(
		settingRow("Cache Prompt", boolToString(config.server.cachePrompt)));
	settings.push_back(
		settingRow("Metrics", boolToString(config.server.metrics)));

	// Load Settings
	settings.push_back(categoryHeader("Load Settings"));
	settings.push_back(settingRow(
		"Model Path",
		config.load.modelPath.empty() ? "Not set" : config.load.modelPath));
	settings.push_back(settingRow("GPU Layers", config.load.ngpuLayers));
	settings.push_back(settingRow("Context Size",
								  config.load.ctxSize == 0
									  ? "default"
									  : std::to_string(config.load.ctxSize)));
	settings.push_back(
		settingRow("Batch Size", std::to_string(config.load.batchSize)));
	settings.push_back(settingRow("Flash Attention", config.load.flashAttn));
	settings.push_back(settingRow("Memory Map", boolToString(config.load.mmap)));
	settings.push_back(
		settingRow("Memory Lock", boolToString(config.load.mlock)));
	settings.push_back(
		settingRow("Fit to Memory", boolToString(config.load.fit)));

	// Inference Settings
	settings.push_back(categoryHeader("Inference Settings"));
	settings.push_back(
		settingRow("Temperature", std::to_string(config.inference.temperature)));
	settings.push_back(
		settingRow("Top P", std::to_string(config.inference.topP)));
	settings.push_back(
		settingRow("Top K", std::to_string(config.inference.topK)));
	settings.push_back(
		settingRow("Min P", std::to_string(config.inference.minP)));
	settings.push_back(
		settingRow("Repeat Penalty",
				   std::to_string(config.inference.repeatPenalty)));
	settings.push_back(
		settingRow("Presence Penalty",
				   std::to_string(config.inference.presencePenalty)));
	settings.push_back(
		settingRow("Frequency Penalty",
				   std::to_string(config.inference.frequencyPenalty)));
	settings.push_back(
		settingRow("Max Tokens",
				   config.inference.nPredict == -1
					   ? "unlimited"
					   : std::to_string(config.inference.nPredict)));

	// UI Settings
	settings.push_back(categoryHeader("UI Settings"));
	settings.push_back(settingRow("Theme", config.ui.theme));
	settings.push_back(settingRow("Default Tab",
								  config.ui.defaultTab == 0	  ? "Settings"
								  : config.ui.defaultTab == 1 ? "Server Log"
															  : "Terminal"));
	settings.push_back(settingRow("Show System Panel",
								  boolToString(config.ui.showSystemPanel)));
	settings.push_back(
		settingRow("Refresh Rate",
				   std::to_string(config.ui.refreshRateMs) + "ms"));

	// Terminal Settings
	settings.push_back(categoryHeader("Terminal Settings"));
	settings.push_back(settingRow("Default Shell",
								  config.terminal.defaultShell.empty()
									  ? "system default"
									  : config.terminal.defaultShell));
	settings.push_back(settingRow("Initial Command",
								  config.terminal.initialCommand.empty()
									  ? "none"
									  : config.terminal.initialCommand));
	settings.push_back(settingRow("Working Directory",
								  config.terminal.workingDirectory.empty()
									  ? "current"
									  : config.terminal.workingDirectory));
	settings.push_back(
		settingRow("Default Cols", std::to_string(config.terminal.defaultCols)));
	settings.push_back(
		settingRow("Default Rows", std::to_string(config.terminal.defaultRows)));

	return vbox(settings) | flex;
}
