/**
 * @file modelInfoPanel.cpp
 * @brief Implementation of ModelInfoPanel.
 *
 * Stateless panel that renders model metrics from ModelInfoMonitor.
 * Provides a third column view of llama-server model statistics.
 */

#include "modelInfoPanel.h"
#include "modelInfoMonitor.h"

#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <sstream>

using namespace ftxui;

namespace {
std::string formatDouble(double value, int precision)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(precision) << value;
	return oss.str();
}

std::string formatNumber(uint64_t value)
{
	// Manual thousand separator
	std::string result;
	std::string num = std::to_string(value);
	int count = 0;
	for (int i = static_cast<int>(num.size()) - 1; i >= 0; --i) {
		if (count > 0 && count % 3 == 0) {
			result = ',' + result;
		}
		result = num[i] + result;
		++count;
	}
	return result.empty() ? "0" : result;
}

std::string getStatusString(const ModelInfo &info)
{
	if (!info.isServerRunning || !info.isModelLoaded) {
		return "N/A";
	}
	return info.isIdle ? "Idle" : "Processing";
}
} // namespace

std::string ModelInfoPanel::formatDouble(double value, int precision)
{
	return ::formatDouble(value, precision);
}

std::string ModelInfoPanel::formatNumber(uint64_t value)
{
	return ::formatNumber(value);
}

std::string ModelInfoPanel::getStatusString(const ModelInfo &info)
{
	return ::getStatusString(info);
}

Element ModelInfoPanel::render()
{
	auto info = ModelInfoMonitor::instance().getStats();

	// Format values
	std::string modelName = info.isServerRunning ? info.loadedModel : "N/A";
	if (!info.isModelLoaded && info.isServerRunning) {
		modelName = "Model: None";
	} else if (!info.isServerRunning) {
		modelName = "Server: Offline";
	}

	std::string genTokPerSec = info.isModelLoaded
								   ? formatDouble(info.generationTokensPerSec, 1)
								   : "N/A";
	std::string procTokPerSec =
		info.isModelLoaded ? formatDouble(info.processingTokensPerSec, 1)
						   : "N/A";
	std::string promptTokens =
		info.isModelLoaded ? formatNumber(info.totalPromptTokens) : "N/A";
	std::string generatedTokens =
		info.isModelLoaded ? formatNumber(info.totalGenerationTokens) : "N/A";
	std::string status = getStatusString(info);

	// Build column
	return vbox({
		// Model name
		hbox({
			text("Model: ") | bold,
			text(modelName),
		}),
		separatorLight(),
		// Generation tokens/s
		hbox({
			text("Gen: ") | bold,
			text(genTokPerSec + " tok/s"),
		}),
		separatorLight(),
		// Processing tokens/s
		hbox({
			text("Proc: ") | bold,
			text(procTokPerSec + " tok/s"),
		}),
		separatorLight(),
		// Prompt tokens total
		hbox({
			text("Prompt: ") | bold,
			text(promptTokens + " tok"),
		}),
		separatorLight(),
		// Generation tokens total
		hbox({
			text("Generated: ") | bold,
			text(generatedTokens + " tok"),
		}),
		separatorLight(),
		// Status
		hbox({
			text("Status: ") | bold,
			text(status) | color(info.isIdle ? Color::Green : Color::Yellow),
		}),
	});
}