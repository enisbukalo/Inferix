/**
 * @file config_manager.cpp
 * @brief Implementation of the ConfigManager singleton.
 *
 * This file implements the ConfigManager class, which provides a
 * thread-safe singleton for loading and saving application configuration.
 *
 * The configuration is stored as JSON at:
 * - Linux/macOS: ~/.inferix/config.json
 * - Windows: %USERPROFILE%\.inferix\config.json
 *
 * The Load() method uses graceful degradation:
 * 1. Creates config directory if it doesn't exist
 * 2. Attempts to parse existing config file
 * 3. Falls back to default values if file is missing or invalid
 * 4. Saves default config if no file existed
 *
 * The Save() method serializes the current config to pretty-printed JSON
 * with 4-space indentation for human readability.
 *
 * @note Uses Meyers singleton pattern for thread-safe, lazy initialization.
 * @note The singleton is destroyed at program termination, but config
 *       should be saved explicitly before exit if modifications were made.
 *
 * @see config_manager.h for API documentation
 * @see config.h for UserConfig structure documentation
 */

#include "config_manager.h"

#include <filesystem>
#include <fstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

ConfigManager &ConfigManager::Instance()
{
	static ConfigManager instance;
	return instance;
}

std::string ConfigManager::GetConfigDir()
{
#ifdef _WIN32
	const char *home = std::getenv("USERPROFILE");
#else
	const char *home = std::getenv("HOME");
#endif
	if (!home)
		return ".inferix";
	return std::string(home) + "/.inferix";
}

std::string ConfigManager::GetConfigFilePath()
{
	return GetConfigDir() + "/config.json";
}

bool ConfigManager::Load()
{
	std::string config_dir = GetConfigDir();
	std::string config_file = GetConfigFilePath();

	/*
	 * Load algorithm:
	 * 1. Create config directory if it doesn't exist
	 * 2. Try to open and parse existing config file
	 * 3. If file exists but is invalid JSON, fall back to defaults
	 * 4. If file doesn't exist, use defaults and save them
	 *
	 * This ensures the application always has valid configuration,
	 * even if the config file is corrupted or missing.
	 */

	// Create config directory if it doesn't exist
	try {
		fs::create_directories(config_dir);
	} catch (const std::exception &) {
		// If we can't create the directory, use defaults
		config_ = Config::UserConfig{};
		loaded_ = true;
		return true;
	}

	// Try to read existing config file
	std::ifstream file(config_file);
	if (file.is_open()) {
		try {
			json j = json::parse(file);
			config_ = j.get<Config::UserConfig>();
		} catch (const std::exception &) {
			// Invalid JSON, use defaults
			config_ = Config::UserConfig{};
		}
	} else {
		// No config file, use defaults and save them
		config_ = Config::UserConfig{};
		Save();
	}

	loaded_ = true;
	return true;
}

bool ConfigManager::Save()
{
	std::string config_dir = GetConfigDir();
	std::string config_file = GetConfigFilePath();

	try {
		fs::create_directories(config_dir);
	} catch (const std::exception &) {
		return false;
	}

	std::ofstream file(config_file);
	if (!file.is_open())
		return false;

	json j = config_;
	file << j.dump(4) << std::endl;
	return true;
}

const Config::UserConfig &ConfigManager::GetConfig() const
{
	return config_;
}

Config::UserConfig &ConfigManager::GetConfig()
{
	return config_;
}

bool ConfigManager::IsLoaded() const
{
	return loaded_;
}
