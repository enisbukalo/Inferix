/**
 * @file config.cpp
 * @brief JSON serialization functions for configuration structures.
 *
 * Implements to_json/from_json functions for all Config structures
 * using nlohmann::json. These functions enable automatic serialization
 * and deserialization of the UserConfig structure.
 *
 * The serialization format is:
 * @code
 * {
 *   "server": { ... },
 *   "load": { ... },
 *   "inference": { ... },
 *   "ui": { ... },
 *   "terminal": { ... },
 *   "presets": [ ... ]
 * }
 * @endcode
 */

#include "config.h"

using json = nlohmann::json;

namespace Config {

// ============================================================================
// ServerSettings serialization
// ============================================================================

void to_json(json &j, const ServerSettings &v)
{
	j["host"] = v.host;
	j["port"] = v.port;
	j["apiKey"] = v.apiKey;
	j["apiKeyFile"] = v.apiKeyFile;
	j["timeout"] = v.timeout;
	j["threadsHttp"] = v.threadsHttp;

	j["sslKeyFile"] = v.sslKeyFile;
	j["sslCertFile"] = v.sslCertFile;

	j["path"] = v.path;
	j["apiPrefix"] = v.apiPrefix;
	j["mediaPath"] = v.mediaPath;

	j["alias"] = v.alias;
	j["webui"] = v.webui;
	j["embedding"] = v.embedding;
	j["reranking"] = v.reranking;
	j["contBatching"] = v.contBatching;
	j["cachePrompt"] = v.cachePrompt;
	j["cacheReuse"] = v.cacheReuse;
	j["contextShift"] = v.contextShift;
	j["warmup"] = v.warmup;
	j["jinja"] = v.jinja;
	j["prefillAssistant"] = v.prefillAssistant;
	j["slotPromptSimilarity"] = v.slotPromptSimilarity;
	j["sleepIdleSeconds"] = v.sleepIdleSeconds;

	j["metrics"] = v.metrics;
	j["props"] = v.props;
	j["slots"] = v.slots;
	j["slotSavePath"] = v.slotSavePath;
}

void from_json(const json &j, ServerSettings &v)
{
	v.host = j.value("host", v.host);
	v.port = j.value("port", v.port);
	v.apiKey = j.value("apiKey", v.apiKey);
	v.apiKeyFile = j.value("apiKeyFile", v.apiKeyFile);
	v.timeout = j.value("timeout", v.timeout);
	v.threadsHttp = j.value("threadsHttp", v.threadsHttp);

	v.sslKeyFile = j.value("sslKeyFile", v.sslKeyFile);
	v.sslCertFile = j.value("sslCertFile", v.sslCertFile);

	v.path = j.value("path", v.path);
	v.apiPrefix = j.value("apiPrefix", v.apiPrefix);
	v.mediaPath = j.value("mediaPath", v.mediaPath);

	v.alias = j.value("alias", v.alias);
	v.webui = j.value("webui", v.webui);
	v.embedding = j.value("embedding", v.embedding);
	v.reranking = j.value("reranking", v.reranking);
	v.contBatching = j.value("contBatching", v.contBatching);
	v.cachePrompt = j.value("cachePrompt", v.cachePrompt);
	v.cacheReuse = j.value("cacheReuse", v.cacheReuse);
	v.contextShift = j.value("contextShift", v.contextShift);
	v.warmup = j.value("warmup", v.warmup);
	v.jinja = j.value("jinja", v.jinja);
	v.prefillAssistant = j.value("prefillAssistant", v.prefillAssistant);
	v.slotPromptSimilarity =
		j.value("slotPromptSimilarity", v.slotPromptSimilarity);
	v.sleepIdleSeconds = j.value("sleepIdleSeconds", v.sleepIdleSeconds);

	v.metrics = j.value("metrics", v.metrics);
	v.props = j.value("props", v.props);
	v.slots = j.value("slots", v.slots);
	v.slotSavePath = j.value("slotSavePath", v.slotSavePath);
}

// ============================================================================
// LoadSettings serialization
// ============================================================================

void to_json(json &j, const LoadSettings &v)
{
	j["modelPath"] = v.modelPath;
	j["modelUrl"] = v.modelUrl;
	j["hfRepo"] = v.hfRepo;
	j["hfFile"] = v.hfFile;
	j["hfToken"] = v.hfToken;

	j["ngpuLayers"] = v.ngpuLayers;
	j["splitMode"] = v.splitMode;
	j["tensorSplit"] = v.tensorSplit;
	j["mainGpu"] = v.mainGpu;

	j["ctxSize"] = v.ctxSize;
	j["batchSize"] = v.batchSize;
	j["ubatchSize"] = v.ubatchSize;
	j["parallel"] = v.parallel;

	j["cacheTypeK"] = v.cacheTypeK;
	j["cacheTypeV"] = v.cacheTypeV;
	j["kvOffload"] = v.kvOffload;

	j["flashAttn"] = v.flashAttn;
	j["mlock"] = v.mlock;
	j["mmap"] = v.mmap;

	j["threads"] = v.threads;
	j["threadsBatch"] = v.threadsBatch;

	j["lora"] = v.lora;
	j["mmproj"] = v.mmproj;

	j["modelDraft"] = v.modelDraft;
	j["draftMax"] = v.draftMax;

	j["chatTemplate"] = v.chatTemplate;
	j["reasoningFormat"] = v.reasoningFormat;

	j["fit"] = v.fit;
}

void from_json(const json &j, LoadSettings &v)
{
	v.modelPath = j.value("modelPath", v.modelPath);
	v.modelUrl = j.value("modelUrl", v.modelUrl);
	v.hfRepo = j.value("hfRepo", v.hfRepo);
	v.hfFile = j.value("hfFile", v.hfFile);
	v.hfToken = j.value("hfToken", v.hfToken);

	v.ngpuLayers = j.value("ngpuLayers", v.ngpuLayers);
	v.splitMode = j.value("splitMode", v.splitMode);
	v.tensorSplit = j.value("tensorSplit", v.tensorSplit);
	v.mainGpu = j.value("mainGpu", v.mainGpu);

	v.ctxSize = j.value("ctxSize", v.ctxSize);
	v.batchSize = j.value("batchSize", v.batchSize);
	v.ubatchSize = j.value("ubatchSize", v.ubatchSize);
	v.parallel = j.value("parallel", v.parallel);

	v.cacheTypeK = j.value("cacheTypeK", v.cacheTypeK);
	v.cacheTypeV = j.value("cacheTypeV", v.cacheTypeV);
	v.kvOffload = j.value("kvOffload", v.kvOffload);

	v.flashAttn = j.value("flashAttn", v.flashAttn);
	v.mlock = j.value("mlock", v.mlock);
	v.mmap = j.value("mmap", v.mmap);

	v.threads = j.value("threads", v.threads);
	v.threadsBatch = j.value("threadsBatch", v.threadsBatch);

	v.lora = j.value("lora", v.lora);
	v.mmproj = j.value("mmproj", v.mmproj);

	v.modelDraft = j.value("modelDraft", v.modelDraft);
	v.draftMax = j.value("draftMax", v.draftMax);

	v.chatTemplate = j.value("chatTemplate", v.chatTemplate);
	v.reasoningFormat = j.value("reasoningFormat", v.reasoningFormat);

	v.fit = j.value("fit", v.fit);
}

// ============================================================================
// InferenceSettings serialization
// ============================================================================

void to_json(json &j, const InferenceSettings &v)
{
	j["nPredict"] = v.nPredict;
	j["samplers"] = v.samplers;

	j["seed"] = v.seed;
	j["temperature"] = v.temperature;
	j["topK"] = v.topK;
	j["topP"] = v.topP;
	j["minP"] = v.minP;
	j["topNsigma"] = v.topNsigma;
	j["typicalP"] = v.typicalP;

	j["xtcProbability"] = v.xtcProbability;
	j["xtcThreshold"] = v.xtcThreshold;

	j["repeatLastN"] = v.repeatLastN;
	j["repeatPenalty"] = v.repeatPenalty;
	j["presencePenalty"] = v.presencePenalty;
	j["frequencyPenalty"] = v.frequencyPenalty;

	j["dryMultiplier"] = v.dryMultiplier;
	j["dryBase"] = v.dryBase;
	j["dryAllowedLength"] = v.dryAllowedLength;
	j["dryPenaltyLastN"] = v.dryPenaltyLastN;

	j["dynatempRange"] = v.dynatempRange;
	j["dynatempExp"] = v.dynatempExp;

	j["mirostat"] = v.mirostat;
	j["mirostatLr"] = v.mirostatLr;
	j["mirostatEnt"] = v.mirostatEnt;

	j["grammar"] = v.grammar;
	j["jsonSchema"] = v.jsonSchema;
}

void from_json(const json &j, InferenceSettings &v)
{
	v.nPredict = j.value("nPredict", v.nPredict);
	v.samplers = j.value("samplers", v.samplers);

	v.seed = j.value("seed", v.seed);
	v.temperature = j.value("temperature", v.temperature);
	v.topK = j.value("topK", v.topK);
	v.topP = j.value("topP", v.topP);
	v.minP = j.value("minP", v.minP);
	v.topNsigma = j.value("topNsigma", v.topNsigma);
	v.typicalP = j.value("typicalP", v.typicalP);

	v.xtcProbability = j.value("xtcProbability", v.xtcProbability);
	v.xtcThreshold = j.value("xtcThreshold", v.xtcThreshold);

	v.repeatLastN = j.value("repeatLastN", v.repeatLastN);
	v.repeatPenalty = j.value("repeatPenalty", v.repeatPenalty);
	v.presencePenalty = j.value("presencePenalty", v.presencePenalty);
	v.frequencyPenalty = j.value("frequencyPenalty", v.frequencyPenalty);

	v.dryMultiplier = j.value("dryMultiplier", v.dryMultiplier);
	v.dryBase = j.value("dryBase", v.dryBase);
	v.dryAllowedLength = j.value("dryAllowedLength", v.dryAllowedLength);
	v.dryPenaltyLastN = j.value("dryPenaltyLastN", v.dryPenaltyLastN);

	v.dynatempRange = j.value("dynatempRange", v.dynatempRange);
	v.dynatempExp = j.value("dynatempExp", v.dynatempExp);

	v.mirostat = j.value("mirostat", v.mirostat);
	v.mirostatLr = j.value("mirostatLr", v.mirostatLr);
	v.mirostatEnt = j.value("mirostatEnt", v.mirostatEnt);

	v.grammar = j.value("grammar", v.grammar);
	v.jsonSchema = j.value("jsonSchema", v.jsonSchema);
}

// ============================================================================
// UISettings serialization
// ============================================================================

void to_json(json &j, const UISettings &v)
{
	j = json{ { "theme", v.theme },
			  { "defaultTab", v.defaultTab },
			  { "showSystemPanel", v.showSystemPanel },
			  { "refreshRateMs", v.refreshRateMs } };
}

void from_json(const json &j, UISettings &v)
{
	v.theme = j.value("theme", v.theme);
	v.defaultTab = j.value("defaultTab", v.defaultTab);
	v.showSystemPanel = j.value("showSystemPanel", v.showSystemPanel);
	v.refreshRateMs = j.value("refreshRateMs", v.refreshRateMs);
}

// ============================================================================
// TerminalSettings serialization
// ============================================================================

void to_json(json &j, const TerminalSettings &v)
{
	j = json{ { "defaultShell", v.defaultShell },
			  { "initialCommand", v.initialCommand },
			  { "workingDirectory", v.workingDirectory },
			  { "defaultCols", v.defaultCols },
			  { "defaultRows", v.defaultRows } };
}

void from_json(const json &j, TerminalSettings &v)
{
	v.defaultShell = j.value("defaultShell", v.defaultShell);
	v.initialCommand = j.value("initialCommand", v.initialCommand);
	v.workingDirectory = j.value("workingDirectory", v.workingDirectory);
	v.defaultCols = j.value("defaultCols", v.defaultCols);
	v.defaultRows = j.value("defaultRows", v.defaultRows);
}

// ============================================================================
// ModelPreset serialization
// ============================================================================

void to_json(json &j, const ModelPreset &v)
{
	j["name"] = v.name;
	j["model"] = v.model;
	j["inference"] = v.inference;
}

void from_json(const json &j, ModelPreset &v)
{
	v.name = j.value("name", v.name);
	v.model = j.value("model", v.model);
	if (j.contains("inference"))
		v.inference = j["inference"].get<InferenceSettings>();
}

// ============================================================================
// UserConfig serialization (main container)
// ============================================================================

void to_json(json &j, const UserConfig &v)
{
	j["server"] = v.server;
	j["load"] = v.load;
	j["inference"] = v.inference;
	j["ui"] = v.ui;
	j["terminal"] = v.terminal;
	j["presets"] = v.presets;
}

void from_json(const json &j, UserConfig &v)
{
	if (j.contains("server"))
		v.server = j["server"].get<ServerSettings>();
	if (j.contains("load"))
		v.load = j["load"].get<LoadSettings>();
	if (j.contains("inference"))
		v.inference = j["inference"].get<InferenceSettings>();
	if (j.contains("ui"))
		v.ui = j["ui"].get<UISettings>();
	if (j.contains("terminal"))
		v.terminal = j["terminal"].get<TerminalSettings>();
	if (j.contains("presets"))
		v.presets = j["presets"].get<std::vector<ModelPreset>>();
}

} // namespace Config
