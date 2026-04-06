#include "llamaServerProcess.h"
#include "modelsIni.h"
#include <filesystem>
#include <spdlog/spdlog.h>
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

	// Router mode: use --models-preset to point to models.ini
	// The INI file defines all available models
	std::string iniPath = ModelsIni::instance().getPath();
	if (!iniPath.empty() && std::filesystem::exists(iniPath)) {
		args.push_back("--models-preset");
		args.push_back(iniPath);
	} else if (!iniPath.empty()) {
		spdlog::warn("models.ini path set but file does not exist: {}", iniPath);
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
	// Network
	if (!server.host.empty()) {
		args.push_back("--host");
		args.push_back(server.host);
	}
	args.push_back("--port");
	args.push_back(std::to_string(server.port));

	// Authentication
	if (!server.apiKey.empty()) {
		args.push_back("--api-key");
		args.push_back(server.apiKey);
	}
	if (!server.apiKeyFile.empty()) {
		args.push_back("--api-key-file");
		args.push_back(server.apiKeyFile);
	}
	if (server.timeout > 0) {
		args.push_back("--timeout");
		args.push_back(std::to_string(server.timeout));
	}
	if (server.threadsHttp > 0) {
		args.push_back("--threads-http");
		args.push_back(std::to_string(server.threadsHttp));
	}
	if (server.reusePort) {
		args.push_back("--reuse-port");
	}

	// SSL/TLS
	if (!server.sslKeyFile.empty()) {
		args.push_back("--ssl-key-file");
		args.push_back(server.sslKeyFile);
	}
	if (!server.sslCertFile.empty()) {
		args.push_back("--ssl-cert-file");
		args.push_back(server.sslCertFile);
	}

	// Static file serving
	if (!server.path.empty()) {
		args.push_back("--path");
		args.push_back(server.path);
	}
	if (!server.apiPrefix.empty()) {
		args.push_back("--api-prefix");
		args.push_back(server.apiPrefix);
	}
	if (!server.mediaPath.empty()) {
		args.push_back("--media-path");
		args.push_back(server.mediaPath);
	}

	// Server behavior
	if (!server.alias.empty()) {
		args.push_back("--alias");
		args.push_back(server.alias);
	}
	args.push_back(server.webui ? "--webui" : "--no-webui");
	if (!server.webuiConfig.empty()) {
		args.push_back("--webui-config");
		args.push_back(server.webuiConfig);
	}
	if (!server.webuiConfigFile.empty()) {
		args.push_back("--webui-config-file");
		args.push_back(server.webuiConfigFile);
	}
	args.push_back(server.webuiMcpProxy ? "--webui-mcp-proxy"
										: "--no-webui-mcp-proxy");
	if (!server.tools.empty()) {
		args.push_back("--tools");
		args.push_back(server.tools);
	}
	if (server.embedding) {
		args.push_back("--embedding");
	}
	if (server.reranking) {
		args.push_back("--rerank");
	}
	args.push_back(server.contBatching ? "--cont-batching"
									   : "--no-cont-batching");
	args.push_back(server.cachePrompt ? "--cache-prompt" : "--no-cache-prompt");
	if (server.cacheReuse > 0) {
		args.push_back("--cache-reuse");
		args.push_back(std::to_string(server.cacheReuse));
	}
	args.push_back(server.contextShift ? "--context-shift"
									   : "--no-context-shift");
	args.push_back(server.warmup ? "--warmup" : "--no-warmup");
	args.push_back(server.jinja ? "--jinja" : "--no-jinja");
	args.push_back(server.prefillAssistant ? "--prefill-assistant"
										   : "--no-prefill-assistant");
	if (server.slotPromptSimilarity >= 0.0 &&
		server.slotPromptSimilarity <= 1.0) {
		args.push_back("--slot-prompt-similarity");
		args.push_back(std::to_string(server.slotPromptSimilarity));
	}
	if (server.sleepIdleSeconds >= 0) {
		args.push_back("--sleep-idle-seconds");
		args.push_back(std::to_string(server.sleepIdleSeconds));
	}

	// Endpoints
	if (server.metrics) {
		args.push_back("--metrics");
	}
	if (server.props) {
		args.push_back("--props");
	}
	args.push_back(server.slots ? "--slots" : "--no-slots");
	if (!server.slotSavePath.empty()) {
		args.push_back("--slot-save-path");
		args.push_back(server.slotSavePath);
	}

	// Log file - use llama-server's built-in logging with timestamps
	args.push_back("--log-file");
	args.push_back(ConfigManager::getLogsDir() + "/llama-server.log");
	args.push_back("--log-timestamps");
	args.push_back("--log-prefix");
	args.push_back("--log-colors");
	args.push_back("off");

	return args;
}