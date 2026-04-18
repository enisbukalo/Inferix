/**
 * @file modelInfoMonitor.cpp
 * @brief Implementation of ModelInfoMonitor singleton.
 *
 * Polls /slots endpoint at 1Hz to detect state changes.
 * On transition from processing to idle, queries /metrics once.
 * Thread-safe via mutex-protected access.
 */

#include "modelInfoMonitor.h"
#include "configManager.h"
#include "httpClient.h"
#include "llamaServerProcess.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <sstream>

namespace {
constexpr auto POLL_INTERVAL = std::chrono::seconds(1);
constexpr auto HTTP_TIMEOUT_SECONDS = 5;

std::string getServerAddress()
{
	auto &config = ConfigManager::instance().getConfig();
	return "http://" + config.server.host + ":" +
		   std::to_string(config.server.port);
}
} // namespace

ModelInfoMonitor::ModelInfoMonitor()
	: m_running(false), m_wasProcessing(false), m_forceUnloaded(false)
{
	m_modelInfo = ModelInfo{};
}

ModelInfoMonitor::~ModelInfoMonitor()
{
	stop();
}

void ModelInfoMonitor::start()
{
	if (m_running.load())
		return;

	m_running = true;
	m_pollThread = std::thread(&ModelInfoMonitor::pollLoop, this);
}

void ModelInfoMonitor::stop()
{
	if (!m_running.load())
		return;

	m_running = false;
	if (m_pollThread.joinable()) {
		m_pollThread.join();
	}
}

ModelInfo ModelInfoMonitor::getStats() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_modelInfo;
}

void ModelInfoMonitor::setUnloaded()
{
	m_forceUnloaded.store(true);
	m_cachedModel.clear();
}

void ModelInfoMonitor::clearForceUnloaded()
{
	m_forceUnloaded.store(false);
}

bool ModelInfoMonitor::isProcessing(const std::string &slotJson)
{
	// Look for "is_processing":true in the slots response
	// Example:
	// [{"id":0,"n_ctx":9216,"speculative":false,"is_processing":false},...]
	return slotJson.find("\"is_processing\":true") != std::string::npos;
}

void ModelInfoMonitor::pollLoop()
{
	while (m_running) {
		ModelInfo info;

		// Check if server is healthy
		if (!LlamaServerProcess::instance().isServerHealthy()) {
			spdlog::debug("ModelInfoMonitor: server not healthy");
			info.isServerRunning = false;
			info.isModelLoaded = false;
			info.loadedModel = "Server: Offline";
			info.activeRequestCount = 0;
			info.isIdle = true;
			m_cachedModel.clear(); // Clear cached model on offline
		} else {
			info.isServerRunning = true;
			spdlog::debug("ModelInfoMonitor: server is healthy");

			// If we intentionally unloaded, skip all model-specific queries
			// to avoid triggering the server to reload the model
			if (m_forceUnloaded.load()) {
				spdlog::debug(
					"ModelInfoMonitor: force unloaded, skipping model queries");
				m_cachedModel.clear();
			}

			// Get currently loaded model - only refresh if cache is empty
			// (e.g., first poll or after server came back online)
			if (m_cachedModel.empty() && !m_forceUnloaded.load()) {
				m_cachedModel =
					LlamaServerProcess::instance().getLoadedModelPath();
				spdlog::debug("ModelInfoMonitor: cached model: {}",
							  m_cachedModel);
			}

			if (m_cachedModel.empty()) {
				info.isModelLoaded = false;
				info.loadedModel = "Model: None";
				info.activeRequestCount = 0;
				info.isIdle = true;
			} else {
				info.isModelLoaded = true;
				info.loadedModel = m_cachedModel;

				// Get slot status to check if processing - pass model name
				auto slotStatus =
					LlamaServerProcess::instance().getSlotStatus(m_cachedModel);
				spdlog::debug("ModelInfoMonitor: slot status length: {}",
							  slotStatus.size());
				bool currentlyProcessing = isProcessing(slotStatus);

				// Count active requests - look for "is_processing":true
				// occurrences
				int activeCount = 0;
				size_t pos = 0;
				while ((pos = slotStatus.find("\"is_processing\":true", pos)) !=
					   std::string::npos) {
					activeCount++;
					pos += 18;
				}
				info.activeRequestCount = activeCount;
				info.isIdle = !currentlyProcessing;

				// Detect transition: was processing, now idle -> fetch metrics
				// once
				spdlog::debug(
					"ModelInfoMonitor: wasProcessing={}, currentlyProcessing={}",
					m_wasProcessing,
					currentlyProcessing);
				if (m_wasProcessing && !currentlyProcessing) {
					spdlog::debug("ModelInfoMonitor: slot transitioned to idle, "
								  "fetching metrics");
					auto metrics = fetchMetricsOnce(m_cachedModel);
					spdlog::debug("ModelInfoMonitor: metrics - genTokPerSec={}, "
								  "procTokPerSec={}, "
								  "promptTokens={}, generatedTokens={}",
								  metrics.generationTokensPerSec,
								  metrics.processingTokensPerSec,
								  metrics.totalPromptTokens,
								  metrics.totalGenerationTokens);
					// Update with new metrics
					info.generationTokensPerSec = metrics.generationTokensPerSec;
					info.processingTokensPerSec = metrics.processingTokensPerSec;
					info.totalPromptTokens = metrics.totalPromptTokens;
					info.totalGenerationTokens = metrics.totalGenerationTokens;
				} else {
					// Keep previous values while processing
					std::lock_guard<std::mutex> lock(m_mutex);
					info.generationTokensPerSec =
						m_modelInfo.generationTokensPerSec;
					info.processingTokensPerSec =
						m_modelInfo.processingTokensPerSec;
					info.totalPromptTokens = m_modelInfo.totalPromptTokens;
					info.totalGenerationTokens =
						m_modelInfo.totalGenerationTokens;
				}

				m_wasProcessing = currentlyProcessing;
			}
		}

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_modelInfo = info;
		}

		std::this_thread::sleep_for(POLL_INTERVAL);
	}
}

ModelInfo ModelInfoMonitor::fetchMetricsOnce(const std::string &modelName)
{
	ModelInfo info;
	info.generationTokensPerSec = 0.0;
	info.processingTokensPerSec = 0.0;
	info.totalPromptTokens = 0;
	info.totalGenerationTokens = 0;

	HttpClient client;
	client.setTimeout(HTTP_TIMEOUT_SECONDS);

	auto url = getServerAddress() + "/metrics?model=" + modelName;
	auto [success, response] = client.get(url);

	if (!success) {
		spdlog::warn("ModelInfoMonitor: failed to fetch metrics: {}", response);
		return info;
	}

	return parseMetricsResponse(response);
}

ModelInfo ModelInfoMonitor::parseMetricsResponse(const std::string &response)
{
	ModelInfo info;
	info.generationTokensPerSec = 0.0;
	info.processingTokensPerSec = 0.0;
	info.totalPromptTokens = 0;
	info.totalGenerationTokens = 0;

	// Parse Prometheus-style metrics
	// Expected metrics:
	// predicted_tokens_seconds{model="..."} 37.5
	// prompt_tokens_seconds{model="..."} 2038.0
	// prompt_tokens_total{model="..."} 2020
	// tokens_predicted_total{model="..."} 776

	std::istringstream stream(response);
	std::string line;

	while (std::getline(stream, line)) {
		// Skip comments and empty lines
		if (line.empty() || line[0] == '#')
			continue;

		// Parse metric name and value
		// Format: metric_name{labels} value
		size_t spacePos = line.find(' ');
		if (spacePos == std::string::npos)
			continue;

		std::string metricName = line.substr(0, spacePos);
		std::string valueStr = line.substr(spacePos + 1);

		try {
			double value = std::stod(valueStr);

			if (metricName == "predicted_tokens_seconds") {
				info.generationTokensPerSec = value;
			} else if (metricName == "prompt_tokens_seconds") {
				info.processingTokensPerSec = value;
			} else if (metricName == "prompt_tokens_total") {
				info.totalPromptTokens = static_cast<uint64_t>(value);
			} else if (metricName == "tokens_predicted_total") {
				info.totalGenerationTokens = static_cast<uint64_t>(value);
			}
		} catch (const std::exception &e) {
			spdlog::debug("ModelInfoMonitor: failed to parse metric line: {}",
						  line);
		}
	}

	return info;
}