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
	// GPU layers (default is "all" now, so only add if not default)
	if (!load.ngpuLayers.empty() && load.ngpuLayers != "all") {
		args.push_back("-ngl");
		args.push_back(load.ngpuLayers);
	}
	// Split mode
	if (!load.splitMode.empty()) {
		args.push_back("-sm");
		args.push_back(load.splitMode);
	}
	// Tensor split ratios
	if (!load.tensorSplit.empty()) {
		args.push_back("-ts");
		args.push_back(load.tensorSplit);
	}
	// Device priority - set main GPU for priority (e.g., "0" = GPU 0 first, "1"
	// = GPU 1 first)
	if (!load.devicePriority.empty()) {
		args.push_back("-mg");
		args.push_back(load.devicePriority);
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
	// Parallel slots
	if (load.parallel > 0) {
		args.push_back("-np");
		args.push_back(std::to_string(load.parallel));
	}
	// Flash attention
	if (!load.flashAttn.empty() && load.flashAttn != "auto") {
		args.push_back("-fa");
		args.push_back(load.flashAttn);
	}
	// KV cache offload
	if (!load.kvOffload) {
		args.push_back("--no-kv-offload");
	}
	// KV unified
	if (!load.kvUnified) {
		args.push_back("--no-kv-unified");
	}
	// KV cache type K
	if (!load.cacheTypeK.empty() && load.cacheTypeK != "f16") {
		args.push_back("-ctk");
		args.push_back(load.cacheTypeK);
	}
	// KV cache type V
	if (!load.cacheTypeV.empty() && load.cacheTypeV != "f16") {
		args.push_back("-ctv");
		args.push_back(load.cacheTypeV);
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
	// LoRA adapter
	if (!load.lora.empty()) {
		args.push_back("--lora");
		args.push_back(load.lora);
	}
	// Multimodal projector
	if (!load.mmproj.empty()) {
		args.push_back("-mm");
		args.push_back(load.mmproj);
	}
	// Draft model
	if (!load.modelDraft.empty()) {
		args.push_back("-md");
		args.push_back(load.modelDraft);
	}
	// Draft max tokens
	if (load.draftMax > 0) {
		args.push_back("--draft-max");
		args.push_back(std::to_string(load.draftMax));
	}
	// Chat template
	if (!load.chatTemplate.empty()) {
		args.push_back("--chat-template");
		args.push_back(load.chatTemplate);
	}
	// Reasoning format
	if (!load.reasoningFormat.empty()) {
		args.push_back("--reasoning-format");
		args.push_back(load.reasoningFormat);
	}

	// === Inference Settings ===
	// Seed
	if (inference.seed > 0) {
		args.push_back("-s");
		args.push_back(std::to_string(inference.seed));
	}
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

	// Log file - use llama-server's built-in logging with timestamps and colors
	args.push_back("--log-file");
	args.push_back(ConfigManager::getLogsDir() + "/llama-server.log");
	args.push_back("--log-timestamps");
	args.push_back("--log-prefix");
	args.push_back("--log-colors");
	args.push_back("on");

	return args;
}