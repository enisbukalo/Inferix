#pragma once

#include "llamaServerProcess.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

/**
 * @file modelInfoMonitor.h
 * @brief Thread-safe singleton that monitors llama-server state.
 *
 * Polls /slots endpoint at 1Hz to track server state (idle/processing).
 * On transition from processing to idle, queries /metrics once to get
 * final token statistics for the completed request.
 *
 * This approach avoids continuous /metrics polling which is heavy on the server.
 * Only queries metrics when a request completes (transition to idle).
 *
 * Thread-safe: All public methods can be called concurrently.
 */

/**
 * @struct ModelInfo
 * @brief Data structure holding the current model metrics.
 */
struct ModelInfo
{
	std::string loadedModel;		// e.g., "nvidia_Orchestrator-8B-Q6_K_L"
	double generationTokensPerSec;	// tokens predicted per second
	double processingTokensPerSec;	// prompt tokens per second
	uint64_t totalPromptTokens;		// total prompt tokens processed
	uint64_t totalGenerationTokens; // total generation tokens processed
	int activeRequestCount;			// number of active requests
	bool isIdle;					// true if all slots idle
	bool isServerRunning;			// server is healthy
	bool isModelLoaded;				// a model is loaded
};

/**
 * @class ModelInfoMonitor
 * @brief Singleton that monitors llama-server state and fetches metrics on
 * demand.
 *
 * Polls /slots endpoint at 1Hz to detect state changes. When slots transition
 * from processing to idle, queries /metrics once to get final token counts.
 * This is much lighter than continuously polling /metrics.
 *
 * Thread-safe storage uses mutex-protected access.
 */
class ModelInfoMonitor
{
  public:
	/**
	 * @brief Returns the process-wide singleton instance.
	 *
	 * Uses Meyers' singleton pattern for thread-safe lazy initialization.
	 *
	 * @return Reference to the single @c ModelInfoMonitor object.
	 */
	static ModelInfoMonitor &instance()
	{
		static ModelInfoMonitor monitor;
		return monitor;
	}

	/**
	 * @brief Starts the background polling thread.
	 */
	void start();

	/**
	 * @brief Stops the background polling thread.
	 */
	void stop();

	/**
	 * @brief Returns the latest ModelInfo snapshot.
	 *
	 * Thread-safe getter for the current model metrics.
	 *
	 * @return Copy of the latest @c ModelInfo.
	 */
	ModelInfo getStats() const;

  private:
	ModelInfoMonitor();
	~ModelInfoMonitor();

	// Non-copyable
	ModelInfoMonitor(const ModelInfoMonitor &) = delete;
	ModelInfoMonitor &operator=(const ModelInfoMonitor &) = delete;

	/**
	 * @brief Background polling loop.
	 */
	void pollLoop();

	/**
	 * @brief Check if any slot is processing a request.
	 *
	 * @return true if at least one slot is processing, false if all idle
	 */
	bool isProcessing(const std::string &slotJson);

	/**
	 * @brief Fetches metrics once from llama-server.
	 *
	 * @return ModelInfo with the fetched values, or default if failed.
	 */
	ModelInfo fetchMetricsOnce(const std::string &modelName);

	/**
	 * @brief Parses Prometheus-style metrics response.
	 *
	 * @param response Raw response body from /metrics
	 * @return ModelInfo with parsed values.
	 */
	ModelInfo parseMetricsResponse(const std::string &response);

	// Thread-safe state
	mutable std::mutex m_mutex;
	ModelInfo m_modelInfo;

	// Thread control
	std::atomic<bool> m_running;
	std::thread m_pollThread;

	// Track state for transition detection
	bool m_wasProcessing;

	// Cache model path - only refresh when transitioning from offline/empty
	std::string m_cachedModel;
};