#pragma once

#include <cstdint>
#include <string>

/**
 * @struct ModelInfo
 * @brief Data structure holding the current model metrics.
 *
 * Defined here so both IModelInfoMonitor and ModelInfoMonitor can reference it
 * without circular includes.
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
