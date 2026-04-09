#include <gtest/gtest.h>
#include "config/config.h"
#include "config/configManager.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>

using namespace Config;

// Helper to convert config to JSON and back
template <typename T>
nlohmann::json serializeRoundtrip(const T& obj) {
    nlohmann::json j;
    to_json(j, obj);
    return j;
}

template <typename T>
T deserializeRoundtrip(const nlohmann::json& j) {
    T obj;
    from_json(j, obj);
    return obj;
}

// =============================================================================
// ServerSettings Tests
// =============================================================================

TEST(ConfigSerialization, ServerSettings_DefaultRoundtrip) {
    ServerSettings original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<ServerSettings>(j);

    EXPECT_EQ(original.host, restored.host);
    EXPECT_EQ(original.port, restored.port);
    EXPECT_EQ(original.apiKey, restored.apiKey);
    EXPECT_EQ(original.timeout, restored.timeout);
    EXPECT_EQ(original.threadsHttp, restored.threadsHttp);
    EXPECT_EQ(original.webui, restored.webui);
    EXPECT_EQ(original.embedding, restored.embedding);
    EXPECT_EQ(original.reusePort, restored.reusePort);
    EXPECT_EQ(original.webuiMcpProxy, restored.webuiMcpProxy);
    EXPECT_EQ(original.tools, restored.tools);
}

TEST(ConfigSerialization, ServerSettings_ModifiedValues) {
    ServerSettings original;
    original.host = "0.0.0.0";
    original.port = 9090;
    original.apiKey = "secret-key-123";
    original.timeout = 300;
    original.threadsHttp = 4;
    original.reusePort = true;
    original.webui = false;
    original.webuiConfig = R"({"theme":"dark"})";
    original.webuiConfigFile = "/path/to/config.json";
    original.webuiMcpProxy = true;
    original.tools = "all";
    original.embedding = true;
    original.sslKeyFile = "/path/to/key.pem";
    original.sslCertFile = "/path/to/cert.pem";
    original.path = "/var/www/html";
    original.apiPrefix = "/api";
    original.mediaPath = "/path/to/media";
    original.alias = "test-server";
    original.metrics = true;
    original.props = true;
    original.slots = false;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<ServerSettings>(j);

    EXPECT_EQ(restored.host, "0.0.0.0");
    EXPECT_EQ(restored.port, 9090);
    EXPECT_EQ(restored.apiKey, "secret-key-123");
    EXPECT_EQ(restored.timeout, 300);
    EXPECT_EQ(restored.threadsHttp, 4);
    EXPECT_EQ(restored.reusePort, true);
    EXPECT_EQ(restored.webui, false);
    EXPECT_EQ(restored.webuiConfig, R"({"theme":"dark"})");
    EXPECT_EQ(restored.webuiConfigFile, "/path/to/config.json");
    EXPECT_EQ(restored.webuiMcpProxy, true);
    EXPECT_EQ(restored.tools, "all");
    EXPECT_EQ(restored.embedding, true);
    EXPECT_EQ(restored.sslKeyFile, "/path/to/key.pem");
    EXPECT_EQ(restored.sslCertFile, "/path/to/cert.pem");
    EXPECT_EQ(restored.path, "/var/www/html");
    EXPECT_EQ(restored.apiPrefix, "/api");
    EXPECT_EQ(restored.mediaPath, "/path/to/media");
    EXPECT_EQ(restored.alias, "test-server");
    EXPECT_EQ(restored.metrics, true);
    EXPECT_EQ(restored.props, true);
    EXPECT_EQ(restored.slots, false);
}

// =============================================================================
// LoadSettings Tests
// =============================================================================

TEST(ConfigSerialization, LoadSettings_DefaultRoundtrip) {
    LoadSettings original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<LoadSettings>(j);

    EXPECT_EQ(original.modelPath, restored.modelPath);
    EXPECT_EQ(original.ngpuLayers, restored.ngpuLayers);
    EXPECT_EQ(original.ctxSize, restored.ctxSize);
    EXPECT_EQ(original.batchSize, restored.batchSize);
    EXPECT_EQ(original.parallel, restored.parallel);
    EXPECT_EQ(original.flashAttn, restored.flashAttn);
}

TEST(ConfigSerialization, LoadSettings_ModifiedValues) {
    LoadSettings original;
    original.modelPath = "models/mistral-7b.gguf";
    original.modelUrl = "https://example.com/model.gguf";
    original.hfRepo = "mistralai/Mistral-7B-Instruct-v0.1";
    original.hfFile = "mistral-7b-instruct-v0.1-q4_k_m.gguf";
    original.ngpuLayers = "33";
    original.ctxSize = 8192;
    original.batchSize = 4096;
    original.parallel = 8;
    original.flashAttn = "off";
    original.threads = 8;
    original.lora = "adapters/english-lora.bin";
    original.mmproj = "mmproj-model.bin";

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<LoadSettings>(j);

    EXPECT_EQ(restored.modelPath, "models/mistral-7b.gguf");
    EXPECT_EQ(restored.modelUrl, "https://example.com/model.gguf");
    EXPECT_EQ(restored.hfRepo, "mistralai/Mistral-7B-Instruct-v0.1");
    EXPECT_EQ(restored.hfFile, "mistral-7b-instruct-v0.1-q4_k_m.gguf");
    EXPECT_EQ(restored.ngpuLayers, "33");
    EXPECT_EQ(restored.ctxSize, 8192);
    EXPECT_EQ(restored.batchSize, 4096);
    EXPECT_EQ(restored.parallel, 8);
    EXPECT_EQ(restored.flashAttn, "off");
    EXPECT_EQ(restored.threads, 8);
    EXPECT_EQ(restored.lora, "adapters/english-lora.bin");
    EXPECT_EQ(restored.mmproj, "mmproj-model.bin");
}

// =============================================================================
// InferenceSettings Tests
// =============================================================================

TEST(ConfigSerialization, InferenceSettings_DefaultRoundtrip) {
    InferenceSettings original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<InferenceSettings>(j);

    EXPECT_EQ(original.nPredict, restored.nPredict);
    EXPECT_EQ(original.seed, restored.seed);
    EXPECT_FLOAT_EQ(original.temperature, restored.temperature);
    EXPECT_EQ(original.topK, restored.topK);
    EXPECT_FLOAT_EQ(original.topP, restored.topP);
}

TEST(ConfigSerialization, InferenceSettings_ModifiedValues) {
    InferenceSettings original;
    original.nPredict = 512;
    original.seed = 42;
    original.temperature = 0.5;
    original.topK = 20;
    original.topP = 0.9;
    original.minP = 0.1;
    original.repeatLastN = 128;
    original.repeatPenalty = 1.1;
    original.presencePenalty = 0.5;
    original.frequencyPenalty = 0.3;
    original.mirostat = 2;
    original.mirostatLr = 0.05;
    original.mirostatEnt = 6.0;
    original.grammar = R"(root ::= "hello" | "world")";
    original.jsonSchema = R"({"type": "object"})";

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<InferenceSettings>(j);

    EXPECT_EQ(restored.nPredict, 512);
    EXPECT_EQ(restored.seed, 42);
    EXPECT_FLOAT_EQ(restored.temperature, 0.5);
    EXPECT_EQ(restored.topK, 20);
    EXPECT_FLOAT_EQ(restored.topP, 0.9);
    EXPECT_FLOAT_EQ(restored.minP, 0.1);
    EXPECT_EQ(restored.repeatLastN, 128);
    EXPECT_FLOAT_EQ(restored.repeatPenalty, 1.1);
    EXPECT_FLOAT_EQ(restored.presencePenalty, 0.5);
    EXPECT_FLOAT_EQ(restored.frequencyPenalty, 0.3);
    EXPECT_EQ(restored.mirostat, 2);
    EXPECT_FLOAT_EQ(restored.mirostatLr, 0.05);
    EXPECT_FLOAT_EQ(restored.mirostatEnt, 6.0);
    EXPECT_EQ(restored.grammar, R"(root ::= "hello" | "world")");
    EXPECT_EQ(restored.jsonSchema, R"({"type": "object"})");
}

// =============================================================================
// UISettings Tests
// =============================================================================

TEST(ConfigSerialization, UISettings_DefaultRoundtrip) {
    UISettings original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<UISettings>(j);

    EXPECT_EQ(original.theme, restored.theme);
    EXPECT_EQ(original.defaultTab, restored.defaultTab);
    EXPECT_EQ(original.showSystemPanel, restored.showSystemPanel);
    EXPECT_EQ(original.refreshRateMs, restored.refreshRateMs);
}

TEST(ConfigSerialization, UISettings_ModifiedValues) {
    UISettings original;
    original.theme = "dark";
    original.defaultTab = 1;  // int, not string
    original.showSystemPanel = false;
    original.refreshRateMs = 500;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<UISettings>(j);

    EXPECT_EQ(restored.theme, "dark");
    EXPECT_EQ(restored.defaultTab, 1);
    EXPECT_EQ(restored.showSystemPanel, false);
    EXPECT_EQ(restored.refreshRateMs, 500);
}

// =============================================================================
// TerminalSettings Tests
// =============================================================================

TEST(ConfigSerialization, TerminalSettings_DefaultRoundtrip) {
    TerminalSettings original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<TerminalSettings>(j);

    EXPECT_EQ(original.defaultShell, restored.defaultShell);
    EXPECT_EQ(original.initialCommand, restored.initialCommand);
    EXPECT_EQ(original.workingDirectory, restored.workingDirectory);
}

TEST(ConfigSerialization, TerminalSettings_ModifiedValues) {
    TerminalSettings original;
    original.defaultShell = "/bin/zsh";
    original.initialCommand = "htop";
    original.workingDirectory = "/home/user/projects";
    original.defaultCols = 120;
    original.defaultRows = 40;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<TerminalSettings>(j);

    EXPECT_EQ(restored.defaultShell, "/bin/zsh");
    EXPECT_EQ(restored.initialCommand, "htop");
    EXPECT_EQ(restored.workingDirectory, "/home/user/projects");
    EXPECT_EQ(restored.defaultCols, 120);
    EXPECT_EQ(restored.defaultRows, 40);
}

// =============================================================================
// ModelPreset Tests
// =============================================================================

TEST(ConfigSerialization, ModelPreset_DefaultRoundtrip) {
    ModelPreset original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<ModelPreset>(j);

    EXPECT_EQ(original.name, restored.name);
    EXPECT_EQ(original.model, restored.model);
}

TEST(ConfigSerialization, ModelPreset_ModifiedValues) {
    ModelPreset original;
    original.name = "Creative Writer";
    original.model = "models/mistral-7b.gguf";
    original.inference.temperature = 1.2;
    original.inference.topP = 0.95;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<ModelPreset>(j);

    EXPECT_EQ(restored.name, "Creative Writer");
    EXPECT_EQ(restored.model, "models/mistral-7b.gguf");
    EXPECT_FLOAT_EQ(restored.inference.temperature, 1.2);
    EXPECT_FLOAT_EQ(restored.inference.topP, 0.95);
}

// =============================================================================
// TerminalPreset Tests
// =============================================================================

TEST(ConfigSerialization, TerminalPreset_DefaultRoundtrip) {
    TerminalPreset original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<TerminalPreset>(j);

    EXPECT_EQ(original.name, restored.name);
    EXPECT_EQ(original.initialCommand, restored.initialCommand);
}

TEST(ConfigSerialization, TerminalPreset_ModifiedValues) {
    TerminalPreset original;
    original.name = "Monitor";
    original.initialCommand = "htop";
    original.cols = 120;
    original.rows = 40;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<TerminalPreset>(j);

    EXPECT_EQ(restored.name, "Monitor");
    EXPECT_EQ(restored.initialCommand, "htop");
    EXPECT_EQ(restored.cols, 120);
    EXPECT_EQ(restored.rows, 40);
}

// =============================================================================
// DiscoverySettings Tests
// =============================================================================

TEST(ConfigSerialization, DiscoverySettings_DefaultRoundtrip) {
    DiscoverySettings original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<DiscoverySettings>(j);

    EXPECT_EQ(original.modelSearchPath, restored.modelSearchPath);
    EXPECT_EQ(original.fileFilter, restored.fileFilter);
}

TEST(ConfigSerialization, DiscoverySettings_ModifiedValues) {
    DiscoverySettings original;
    original.modelSearchPath = "/path/to/models";
    original.fileFilter = {"mmproj*", "*draft*"};

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<DiscoverySettings>(j);

    EXPECT_EQ(restored.modelSearchPath, "/path/to/models");
    EXPECT_EQ(restored.fileFilter.size(), 2);
}

// =============================================================================
// UserConfig Tests
// =============================================================================

TEST(ConfigSerialization, UserConfig_DefaultRoundtrip) {
    UserConfig original;
    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<UserConfig>(j);

    // Check all top-level fields
    EXPECT_EQ(original.server.host, restored.server.host);
    EXPECT_EQ(original.server.port, restored.server.port);
    EXPECT_EQ(original.load.modelPath, restored.load.modelPath);
    EXPECT_EQ(original.inference.temperature, restored.inference.temperature);
    EXPECT_EQ(original.ui.theme, restored.ui.theme);
    EXPECT_EQ(original.terminal.defaultShell, restored.terminal.defaultShell);
    EXPECT_EQ(original.discovery.modelSearchPath, restored.discovery.modelSearchPath);
    EXPECT_EQ(original.presets.size(), restored.presets.size());
    EXPECT_EQ(original.terminalPresets.size(), restored.terminalPresets.size());
}

TEST(ConfigSerialization, UserConfig_ModifiedValues) {
    UserConfig original;
    original.server.host = "0.0.0.0";
    original.server.port = 8080;
    original.load.modelPath = "models/test.gguf";
    original.load.ngpuLayers = "all";
    original.inference.temperature = 0.7;
    original.inference.nPredict = 256;
    original.ui.theme = "light";
    original.ui.defaultTab = 1;  // int, not string
    original.terminal.defaultShell = "/usr/bin/bash";
    original.discovery.modelSearchPath = "/models";

    ModelPreset preset;
    preset.name = "Test Preset";
    preset.model = "preset-model.gguf";
    preset.inference.temperature = 1.0;
    original.presets.push_back(preset);

    TerminalPreset termPreset;
    termPreset.name = "Shell";
    termPreset.initialCommand = "bash -i";
    original.terminalPresets.push_back(termPreset);

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<UserConfig>(j);

    EXPECT_EQ(restored.server.host, "0.0.0.0");
    EXPECT_EQ(restored.server.port, 8080);
    EXPECT_EQ(restored.load.modelPath, "models/test.gguf");
    EXPECT_EQ(restored.load.ngpuLayers, "all");
    EXPECT_FLOAT_EQ(restored.inference.temperature, 0.7);
    EXPECT_EQ(restored.inference.nPredict, 256);
    EXPECT_EQ(restored.ui.theme, "light");
    EXPECT_EQ(restored.ui.defaultTab, 1);
    EXPECT_EQ(restored.terminal.defaultShell, "/usr/bin/bash");
    EXPECT_EQ(restored.discovery.modelSearchPath, "/models");
    EXPECT_EQ(restored.presets.size(), 1);
    EXPECT_EQ(restored.presets[0].name, "Test Preset");
    EXPECT_EQ(restored.terminalPresets.size(), 1);
    EXPECT_EQ(restored.terminalPresets[0].name, "Shell");
}

// =============================================================================
// Advanced InferenceSettings Fields
// =============================================================================

TEST(ConfigSerialization, InferenceSettings_AdvancedFields) {
    InferenceSettings original;
    original.topNsigma = 2.5;
    original.typicalP = 0.8;
    original.xtcProbability = 0.3;
    original.xtcThreshold = 0.2;
    original.dryMultiplier = 1.5;
    original.dryBase = 2.0;
    original.dryAllowedLength = 3;
    original.dryPenaltyLastN = 128;
    original.dynatempRange = 0.5;
    original.dynatempExp = 1.5;
    original.samplers = "topK;topP;temperature";

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<InferenceSettings>(j);

    EXPECT_FLOAT_EQ(restored.topNsigma, 2.5);
    EXPECT_FLOAT_EQ(restored.typicalP, 0.8);
    EXPECT_FLOAT_EQ(restored.xtcProbability, 0.3);
    EXPECT_FLOAT_EQ(restored.xtcThreshold, 0.2);
    EXPECT_FLOAT_EQ(restored.dryMultiplier, 1.5);
    EXPECT_FLOAT_EQ(restored.dryBase, 2.0);
    EXPECT_EQ(restored.dryAllowedLength, 3);
    EXPECT_EQ(restored.dryPenaltyLastN, 128);
    EXPECT_FLOAT_EQ(restored.dynatempRange, 0.5);
    EXPECT_FLOAT_EQ(restored.dynatempExp, 1.5);
    EXPECT_EQ(restored.samplers, "topK;topP;temperature");
}

TEST(ConfigSerialization, LoadSettings_AllFieldsRoundTrip) {
    LoadSettings original;
    original.modelPath = "/path/model.gguf";
    original.modelUrl = "https://example.com/model.gguf";
    original.hfRepo = "org/repo";
    original.hfFile = "model-q4.gguf";
    original.hfToken = "hf_token123";
    original.ngpuLayers = "40";
    original.splitMode = "layer";
    original.tensorSplit = "0.5,0.5";
    original.devicePriority = "0,1";
    original.ctxSize = 16384;
    original.batchSize = 1024;
    original.ubatchSize = 256;
    original.parallel = 2;
    original.cacheTypeK = "q8_0";
    original.cacheTypeV = "q4_0";
    original.kvOffload = false;
    original.kvUnified = false;
    original.flashAttn = "off";
    original.mlock = true;
    original.mmap = true;
    original.threads = 16;
    original.threadsBatch = 8;
    original.lora = "/lora.bin";
    original.mmproj = "/proj.bin";
    original.modelDraft = "/draft.gguf";
    original.draftMax = 32;
    original.chatTemplate = "chatml";
    original.reasoningFormat = "hidden";
    original.fit = false;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<LoadSettings>(j);

    EXPECT_EQ(restored.modelPath, "/path/model.gguf");
    EXPECT_EQ(restored.splitMode, "layer");
    EXPECT_EQ(restored.tensorSplit, "0.5,0.5");
    EXPECT_EQ(restored.devicePriority, "0,1");
    EXPECT_EQ(restored.ubatchSize, 256);
    EXPECT_EQ(restored.cacheTypeK, "q8_0");
    EXPECT_EQ(restored.cacheTypeV, "q4_0");
    EXPECT_FALSE(restored.kvOffload);
    EXPECT_FALSE(restored.kvUnified);
    EXPECT_TRUE(restored.mlock);
    EXPECT_TRUE(restored.mmap);
    EXPECT_EQ(restored.threadsBatch, 8);
    EXPECT_EQ(restored.modelDraft, "/draft.gguf");
    EXPECT_EQ(restored.draftMax, 32);
    EXPECT_EQ(restored.chatTemplate, "chatml");
    EXPECT_EQ(restored.reasoningFormat, "hidden");
    EXPECT_FALSE(restored.fit);
}

TEST(ConfigSerialization, RoundToTwoDecimals_Precision) {
    InferenceSettings original;
    original.temperature = 0.12345;
    original.topP = 0.99999;
    original.minP = 0.001;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<InferenceSettings>(j);

    // roundToTwoDecimals should round to 2 decimal places
    EXPECT_FLOAT_EQ(restored.temperature, 0.12);
    EXPECT_FLOAT_EQ(restored.topP, 1.0);
    EXPECT_FLOAT_EQ(restored.minP, 0.0);
}

TEST(ConfigSerialization, FromJson_EmptyJson_UsesDefaults) {
    nlohmann::json j = "{}"_json;
    UserConfig config;
    from_json(j, config);

    UserConfig defaults;
    EXPECT_EQ(config.server.host, defaults.server.host);
    EXPECT_EQ(config.server.port, defaults.server.port);
    EXPECT_EQ(config.load.ctxSize, defaults.load.ctxSize);
    EXPECT_EQ(config.inference.temperature, defaults.inference.temperature);
}

TEST(ConfigSerialization, ModelPreset_NestedRoundTrip) {
    ModelPreset original;
    original.name = "full-preset";
    original.model = "/m.gguf";
    original.load.ctxSize = 8192;
    original.load.ngpuLayers = "all";
    original.load.flashAttn = "on";
    original.load.fit = true;
    original.inference.temperature = 0.7;
    original.inference.topP = 0.9;
    original.inference.topK = 50;
    original.inference.nPredict = 1024;

    auto j = serializeRoundtrip(original);
    auto restored = deserializeRoundtrip<ModelPreset>(j);

    EXPECT_EQ(restored.name, "full-preset");
    EXPECT_EQ(restored.model, "/m.gguf");
    EXPECT_EQ(restored.load.ctxSize, 8192);
    EXPECT_EQ(restored.load.ngpuLayers, "all");
    EXPECT_TRUE(restored.load.fit);
    EXPECT_FLOAT_EQ(restored.inference.temperature, 0.7);
    EXPECT_EQ(restored.inference.topK, 50);
    EXPECT_EQ(restored.inference.nPredict, 1024);
}

// =============================================================================
// Missing Keys Test
// =============================================================================

TEST(ConfigSerialization, MissingKeys_UseDefaults) {
    // JSON with only partial fields should use defaults for missing ones
    nlohmann::json j = R"({
        "server": {
            "host": "192.168.1.1"
        },
        "load": {}
    })"_json;

    UserConfig config;
    from_json(j, config);

    // server.host should be from JSON
    EXPECT_EQ(config.server.host, "192.168.1.1");
    // server.port should default
    EXPECT_EQ(config.server.port, 8080);
    // load should use defaults
    EXPECT_EQ(config.load.modelPath, "");
    EXPECT_EQ(config.load.ngpuLayers, "all");
}