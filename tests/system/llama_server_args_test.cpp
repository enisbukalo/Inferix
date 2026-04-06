#include <gtest/gtest.h>
#include "system/llamaServerProcess.h"
#include "config/config.h"
#include <vector>
#include <string>

using namespace Config;

// =============================================================================
// LlamaServerProcess buildCommandArgs Tests
// =============================================================================

TEST(LlamaServerProcess, BuildCommandArgsStartsWithLlamaServer) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_FALSE(args.empty());
    EXPECT_EQ(args[0], "llama-server");
}

TEST(LlamaServerProcess, BuildCommandArgsModelPathIsSecond) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("/path/to/model.gguf", load, inference, server);
    
    // Should have -m flag followed by model path
    ASSERT_GE(args.size(), 3);
    EXPECT_EQ(args[1], "-m");
    EXPECT_EQ(args[2], "/path/to/model.gguf");
}

TEST(LlamaServerProcess, BuildCommandArgsEmptyModelPath) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("", load, inference, server);
    
    // Should still have llama-server but no -m flag
    ASSERT_FALSE(args.empty());
    EXPECT_EQ(args[0], "llama-server");
    // -m should not be present
    for (size_t i = 1; i < args.size(); i++) {
        EXPECT_NE(args[i], "-m");
    }
}

TEST(LlamaServerProcess, BuildCommandArgsWithGpuLayers) {
    LoadSettings load;
    load.ngpuLayers = "33";
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    // Should have -ngl 33
    auto it = std::find(args.begin(), args.end(), "-ngl");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "33");
}

TEST(LlamaServerProcess, BuildCommandArgsWithAllGpuLayers) {
    LoadSettings load;
    load.ngpuLayers = "all";  // default, should NOT add -ngl
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    // -ngl should NOT be present for "all"
    for (size_t i = 1; i < args.size(); i++) {
        EXPECT_NE(args[i], "-ngl");
    }
}

TEST(LlamaServerProcess, BuildCommandArgsWithCtxSize) {
    LoadSettings load;
    load.ctxSize = 8192;
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "-c");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "8192");
}

TEST(LlamaServerProcess, BuildCommandArgsWithBatchSize) {
    LoadSettings load;
    load.batchSize = 4096;
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "-b");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "4096");
}

TEST(LlamaServerProcess, BuildCommandArgsWithParallel) {
    LoadSettings load;
    load.parallel = 4;
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "-np");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "4");
}

TEST(LlamaServerProcess, BuildCommandArgsWithThreads) {
    LoadSettings load;
    load.threads = 8;
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "-t");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "8");
}

TEST(LlamaServerProcess, BuildCommandArgsWithLora) {
    LoadSettings load;
    load.lora = "adapters/english.bin";
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--lora");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "adapters/english.bin");
}

TEST(LlamaServerProcess, BuildCommandArgsWithMmproj) {
    LoadSettings load;
    load.mmproj = "mmproj-model.bin";
    InferenceSettings inference;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "-mm");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "mmproj-model.bin");
}

TEST(LlamaServerProcess, BuildCommandArgsWithInferenceSeed) {
    LoadSettings load;
    InferenceSettings inference;
    inference.seed = 42;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "-s");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "42");
}

TEST(LlamaServerProcess, BuildCommandArgsWithNPredict) {
    LoadSettings load;
    InferenceSettings inference;
    inference.nPredict = 512;
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "-n");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "512");
}

TEST(LlamaServerProcess, BuildCommandArgsWithNPredictNegative) {
    LoadSettings load;
    InferenceSettings inference;
    inference.nPredict = -1;  // should NOT add -n
    ServerSettings server;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    // -n should NOT be present
    for (size_t i = 1; i < args.size(); i++) {
        EXPECT_NE(args[i], "-n");
    }
}

TEST(LlamaServerProcess, BuildCommandArgsWithServerHost) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.host = "127.0.0.1";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--host");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "127.0.0.1");
}

TEST(LlamaServerProcess, BuildCommandArgsWithServerPort) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.port = 8080;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--port");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "8080");
}

TEST(LlamaServerProcess, BuildCommandArgsWithApiKey) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.apiKey = "secret-key";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--api-key");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "secret-key");
}

TEST(LlamaServerProcess, BuildCommandArgsWithApiKeyFile) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.apiKeyFile = "/path/to/keyfile";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--api-key-file");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/path/to/keyfile");
}

TEST(LlamaServerProcess, BuildCommandArgsWithTimeout) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.timeout = 300;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--timeout");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "300");
}

TEST(LlamaServerProcess, BuildCommandArgsWithThreadsHttp) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.threadsHttp = 4;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--threads-http");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "4");
}

TEST(LlamaServerProcess, BuildCommandArgsWithReusePort) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.reusePort = true;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--reuse-port"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithSslKeyFile) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.sslKeyFile = "/path/to/key.pem";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--ssl-key-file");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/path/to/key.pem");
}

TEST(LlamaServerProcess, BuildCommandArgsWithSslCertFile) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.sslCertFile = "/path/to/cert.pem";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--ssl-cert-file");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/path/to/cert.pem");
}

TEST(LlamaServerProcess, BuildCommandArgsWithStaticPath) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.path = "/var/www/html";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--path");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/var/www/html");
}

TEST(LlamaServerProcess, BuildCommandArgsWithApiPrefix) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.apiPrefix = "/api";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--api-prefix");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/api");
}

TEST(LlamaServerProcess, BuildCommandArgsWithMediaPath) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.mediaPath = "/path/to/media";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--media-path");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/path/to/media");
}

TEST(LlamaServerProcess, BuildCommandArgsWithAlias) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.alias = "test-server";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--alias");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "test-server");
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoWebui) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.webui = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-webui"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithWebuiConfig) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.webuiConfig = R"({"theme":"dark"})";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--webui-config");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, R"({"theme":"dark"})");
}

TEST(LlamaServerProcess, BuildCommandArgsWithWebuiConfigFile) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.webuiConfigFile = "/path/to/config.json";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--webui-config-file");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/path/to/config.json");
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoWebuiMcpProxy) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.webuiMcpProxy = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-webui-mcp-proxy"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithTools) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.tools = "all";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--tools");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "all");
}

TEST(LlamaServerProcess, BuildCommandArgsWithEmbedding) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.embedding = true;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--embedding"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithReranking) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.reranking = true;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--rerank"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoContBatching) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.contBatching = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-cont-batching"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoCachePrompt) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.cachePrompt = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-cache-prompt"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithCacheReuse) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.cacheReuse = 10;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--cache-reuse");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "10");
}

TEST(LlamaServerProcess, BuildCommandArgsWithContextShift) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.contextShift = true;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--context-shift"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoWarmup) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.warmup = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-warmup"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoJinja) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.jinja = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-jinja"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoPrefillAssistant) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.prefillAssistant = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-prefill-assistant"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithSlotPromptSimilarity) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.slotPromptSimilarity = 0.5;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--slot-prompt-similarity");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "0.5");
}

TEST(LlamaServerProcess, BuildCommandArgsWithSleepIdleSeconds) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.sleepIdleSeconds = 60;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--sleep-idle-seconds");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "60");
}

TEST(LlamaServerProcess, BuildCommandArgsWithMetrics) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.metrics = true;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--metrics"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithProps) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.props = true;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--props"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithNoSlots) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.slots = false;
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    ASSERT_NE(std::find(args.begin(), args.end(), "--no-slots"), args.end());
}

TEST(LlamaServerProcess, BuildCommandArgsWithSlotSavePath) {
    LoadSettings load;
    InferenceSettings inference;
    ServerSettings server;
    server.slotSavePath = "/path/to/slots";
    
    auto args = LlamaServerProcess::buildCommandArgs("model.gguf", load, inference, server);
    
    auto it = std::find(args.begin(), args.end(), "--slot-save-path");
    ASSERT_NE(it, args.end());
    ++it;
    EXPECT_EQ(*it, "/path/to/slots");
}

TEST(LlamaServerProcess, BuildCommandArgsFullSettings) {
    LoadSettings load;
    load.ngpuLayers = "33";
    load.ctxSize = 8192;
    load.batchSize = 4096;
    load.parallel = 4;
    load.threads = 8;
    load.lora = "adapters/test.bin";
    
    InferenceSettings inference;
    inference.seed = 123;
    inference.nPredict = 256;
    inference.temperature = 0.7f;
    inference.topP = 0.9f;
    inference.topK = 40;
    
    ServerSettings server;
    server.host = "0.0.0.0";
    server.port = 9090;
    
    auto args = LlamaServerProcess::buildCommandArgs("test.gguf", load, inference, server);
    
    // Verify key args are present
    EXPECT_NE(std::find(args.begin(), args.end(), "-m"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "-ngl"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "-c"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "-np"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "-t"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "--lora"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "-s"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "-n"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "--host"), args.end());
    EXPECT_NE(std::find(args.begin(), args.end(), "--port"), args.end());
}

// =============================================================================
// getLogPath Tests
// =============================================================================

TEST(LlamaServerProcess, GetLogPathReturnsValidPath) {
    std::string logPath = LlamaServerProcess::getLogPath();
    
    // Should end with llama-server.log
    EXPECT_TRUE(logPath.find("llama-server.log") != std::string::npos);
}

TEST(LlamaServerProcess, GetLogPathContainsLogsDir) {
    std::string logPath = LlamaServerProcess::getLogPath();
    
    // Should contain "logs" directory
    EXPECT_TRUE(logPath.find("/logs/") != std::string::npos || 
                logPath.find("\\logs\\") != std::string::npos);
}
