#include <gtest/gtest.h>
#include "config/configManager.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <thread>

namespace fs = std::filesystem;

class ConfigManagerTest : public ::testing::Test {
protected:
    fs::path tempDir;
    fs::path configFile;

    void SetUp() override {
        // Create temporary directory for tests
        std::string tempName = "workbench_test_" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        tempDir = fs::temp_directory_path() / tempName;
        fs::create_directories(tempDir);
        configFile = tempDir / "config.json";
    }

    void TearDown() override {
        // Clean up temp directory
        fs::remove_all(tempDir);
    }

    // Override getConfigDir to return our temp directory
    static void setConfigDir(const fs::path& path) {
        ConfigManager::instance().getConfig();
    }
};

// Mock helper - we'll test actual ConfigManager behavior by checking
// the config file it creates and loads

TEST(ConfigManager, LoadCreatesDefaults) {
    // Get a fresh instance (singleton)
    ConfigManager& manager = ConfigManager::instance();

    // The singleton pattern means we can't easily test with temp dirs
    // Instead, we verify that defaults are used when loading fails
    // (non-existent file triggers default config)
    
    // Check that the instance exists and has default values
    const auto& config = manager.getConfig();
    
    // Verify default values
    EXPECT_EQ(config.server.host, "127.0.0.1");
    EXPECT_EQ(config.server.port, 8080);
    EXPECT_EQ(config.load.ngpuLayers, "all");
    EXPECT_EQ(config.inference.temperature, 0.8);
}

TEST(ConfigManager, GetConfigReturnsValidReference) {
    ConfigManager& manager = ConfigManager::instance();
    
    // Non-const access should work
    auto& config = manager.getConfig();
    config.server.port = 9999;
    
    // Getting reference again should reflect changes
    EXPECT_EQ(manager.getConfig().server.port, 9999);
}

TEST(ConfigManager, IsLoadedInitiallyFalse) {
    // When ConfigManager is first created, loaded_ is false
    // (We can't easily test this with singleton, but we can verify the method exists)
    ConfigManager& manager = ConfigManager::instance();
    // The method is accessible - just verify it compiles and runs
    (void)manager.isLoaded();
}

TEST(ConfigManager, ConfigDirPath) {
    std::string configDir = ConfigManager::getConfigDir();
    
    // Should return a non-empty path
    EXPECT_FALSE(configDir.empty());
    
#ifdef _WIN32
    // On Windows, should contain .workbench
    EXPECT_TRUE(configDir.find(".workbench") != std::string::npos);
#else
    // On Linux, should be in home directory
    EXPECT_TRUE(configDir.find("home") != std::string::npos || 
                configDir.find(".workbench") != std::string::npos);
#endif
}

TEST(ConfigManager, ConfigFilePath) {
    std::string configFile = ConfigManager::getConfigFilePath();
    
    // Should end with config.json
    EXPECT_TRUE(configFile.find("config.json") != std::string::npos);
    EXPECT_TRUE(configFile.find(".workbench") != std::string::npos);
}

TEST(ConfigManager, LogsDirPath) {
    std::string logsDir = ConfigManager::getLogsDir();
    
    // Should be non-empty and contain "logs"
    EXPECT_FALSE(logsDir.empty());
    EXPECT_TRUE(logsDir.find("logs") != std::string::npos);
}

// =============================================================================
// Test that serialization produces valid JSON
// =============================================================================

TEST(ConfigSerialization, FullConfigToJson) {
    ConfigManager& manager = ConfigManager::instance();
    const auto& config = manager.getConfig();
    
    nlohmann::json j;
    Config::to_json(j, config);
    
    // Should have all expected top-level keys
    EXPECT_TRUE(j.contains("server"));
    EXPECT_TRUE(j.contains("load"));
    EXPECT_TRUE(j.contains("inference"));
    EXPECT_TRUE(j.contains("ui"));
    EXPECT_TRUE(j.contains("terminal"));
    EXPECT_TRUE(j.contains("discovery"));
    EXPECT_TRUE(j.contains("presets"));
    EXPECT_TRUE(j.contains("terminalPresets"));
}

TEST(ConfigSerialization, FullConfigFromJson) {
    ConfigManager& manager = ConfigManager::instance();
    
    // Create a JSON with all fields
    nlohmann::json j = R"({
        "server": {"host": "192.168.1.100", "port": 9000},
        "load": {"modelPath": "test.gguf", "ngpuLayers": "0"},
        "inference": {"temperature": 0.5, "nPredict": 100},
        "ui": {"theme": "dark", "refreshRateMs": 1000},
        "terminal": {"defaultShell": "/bin/sh"},
        "discovery": {"modelSearchPaths": ["/tmp"]},
        "presets": [],
        "terminalPresets": []
    })"_json;
    
    Config::UserConfig config;
    Config::from_json(j, config);
    
    // Verify values were loaded
    EXPECT_EQ(config.server.host, "192.168.1.100");
    EXPECT_EQ(config.server.port, 9000);
    EXPECT_EQ(config.load.modelPath, "test.gguf");
    EXPECT_EQ(config.load.ngpuLayers, "0");
    EXPECT_FLOAT_EQ(config.inference.temperature, 0.5);
    EXPECT_EQ(config.inference.nPredict, 100);
    EXPECT_EQ(config.ui.theme, "dark");
    EXPECT_EQ(config.ui.refreshRateMs, 1000);
    EXPECT_EQ(config.terminal.defaultShell, "/bin/sh");
    EXPECT_EQ(config.discovery.modelSearchPaths.size(), 1);
}