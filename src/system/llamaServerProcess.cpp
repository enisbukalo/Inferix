#include "llamaServerProcess.h"
#include <vector>

std::vector<std::string>
LlamaServerProcess::buildCommandArgs(const std::string &modelPath,
									 const Config::LoadSettings &load,
									 const Config::InferenceSettings &inference,
									 const Config::ServerSettings &server)
{
	std::vector<std::string> args;

	// Base command
	args.push_back("llama-server");

	// Model (required)
	if (!modelPath.empty()) {
		args.push_back("-m");
		args.push_back(modelPath);
	}

	// === Load Settings ===
	// GPU layers
	if (!load.ngpuLayers.empty() && load.ngpuLayers != "auto") {
		args.push_back("-ngl");
		args.push_back(load.ngpuLayers);
	}
	// Main GPU
	if (load.mainGpu >= 0) {
		args.push_back("-mg");
		args.push_back(std::to_string(load.mainGpu));
	}
	// Context size
	if (load.ctxSize > 0) {
		args.push_back("-c");
		args.push_back(std::to_string(load.ctxSize));
	}
	// Batch size
	if (load.batchSize > 0) {
		args.push_back("-b");
		args.push_back(std::to_string(load.batchSize));
	}
	// Flash attention
	if (!load.flashAttn.empty() && load.flashAttn != "auto") {
		args.push_back("-fa");
		args.push_back(load.flashAttn);
	}
	// Memory map
	args.push_back(load.mmap ? "--mmap" : "--no-mmap");
	// Memory lock
	if (load.mlock) {
		args.push_back("--mlock");
	}
	// Fit to memory
	args.push_back("--fit");
	args.push_back(load.fit ? "on" : "off");
	// Threads
	if (load.threads > 0) {
		args.push_back("-t");
		args.push_back(std::to_string(load.threads));
	}
	// Batch threads
	if (load.threadsBatch > 0) {
		args.push_back("-tb");
		args.push_back(std::to_string(load.threadsBatch));
	}

	// === Inference Settings ===
	// Temperature
	args.push_back("--temp");
	args.push_back(std::to_string(inference.temperature));
	// Top-P
	args.push_back("--top-p");
	args.push_back(std::to_string(inference.topP));
	// Top-K
	args.push_back("--top-k");
	args.push_back(std::to_string(inference.topK));
	// Min-P
	args.push_back("--min-p");
	args.push_back(std::to_string(inference.minP));
	// Repeat penalty
	args.push_back("--repeat-penalty");
	args.push_back(std::to_string(inference.repeatPenalty));
	// Presence penalty
	args.push_back("--presence-penalty");
	args.push_back(std::to_string(inference.presencePenalty));
	// Frequency penalty
	args.push_back("--frequency-penalty");
	args.push_back(std::to_string(inference.frequencyPenalty));
	// nPredict (only if set and not -1)
	if (inference.nPredict > 0) {
		args.push_back("-n");
		args.push_back(std::to_string(inference.nPredict));
	}

	// === Server Settings ===
	if (!server.host.empty()) {
		args.push_back("--host");
		args.push_back(server.host);
	}
	args.push_back("--port");
	args.push_back(std::to_string(server.port));

	return args;
}