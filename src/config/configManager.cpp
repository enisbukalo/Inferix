/**
 * @file configManager.cpp
 * @brief Implementation of the ConfigManager singleton.
 *
 * This file implements the ConfigManager class, which provides a
 * thread-safe singleton for loading and saving application configuration.
 *
 * The configuration is stored as JSON at:
 * - Linux/macOS: ~/.inferix/config.json
 * - Windows: %USERPROFILE%\.inferix\config.json
 *
 * The load() method uses graceful degradation:
 * 1. Creates config directory if it doesn't exist
 * 2. Attempts to parse existing config file
 * 3. Falls back to default values if file is missing or invalid
 * 4. Saves default config if no file existed
 *
 * The save() method serializes the current config to pretty-printed JSON
 * with 4-space indentation for human readability.
 *
 * @note Uses Meyers singleton pattern for thread-safe, lazy initialization.
 * @note The singleton is destroyed at program termination, but config
 *       should be saved explicitly before exit if modifications were made.
 *
 * @see configManager.h for API documentation
 * @see config.h for UserConfig structure documentation
 */

#include "configManager.h"

#include <filesystem>
#include <fstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

ConfigManager &ConfigManager::instance()
{
	static ConfigManager instance;
	return instance;
}

std::string ConfigManager::getConfigDir()
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

std::string ConfigManager::getConfigFilePath()
{
	return getConfigDir() + "/config.json";
}

bool ConfigManager::load()
{
	std::string configDir = getConfigDir();
	std::string configFile = getConfigFilePath();

	/*
	 * Load algorithm:
	 * 1. Create config directory if it doesn't exist
	 * 2. Try to open and parse existing config file
	 * 3. If file exists but is invalid JSON, fall back to defaults
	 * 4. If file doesn't exist, use defaults and create config file
	 *
	 * This ensures the application always has valid configuration,
	 * even if the config file is corrupted or missing.
	 */

	// Create config directory if it doesn't exist
	try {
		fs::create_directories(configDir);
	} catch (const std::exception &) {
		// If we can't create the directory, use defaults
		config_ = Config::UserConfig{};
		loaded_ = true;
		return true;
	}

	// Check if config file exists
	bool fileExists = fs::exists(configFile);

	// Try to read existing config file
	std::ifstream file(configFile);
	if (file.is_open()) {
		try {
			json j = json::parse(file);
			config_ = j.get<Config::UserConfig>();
		} catch (const std::exception &) {
			// Invalid JSON, use defaults
			config_ = Config::UserConfig{};
		}
	} else {
		// No config file, use defaults and create it
		if (!fileExists) {
			// Initialize with default values
			config_ = Config::UserConfig{};
			// Create default config file for the user
			createDefaultConfig();
		} else {
			// File exists but couldn't be opened, use defaults
			config_ = Config::UserConfig{};
		}
	}

	loaded_ = true;
	return true;
}

bool ConfigManager::save()
{
	std::string configDir = getConfigDir();
	std::string configFile = getConfigFilePath();

	try {
		fs::create_directories(configDir);
	} catch (const std::exception &) {
		return false;
	}

	std::ofstream file(configFile);
	if (!file.is_open())
		return false;

	json j = config_;
	file << j.dump(4) << std::endl;
	return true;
}

const Config::UserConfig &ConfigManager::getConfig() const
{
	return config_;
}

Config::UserConfig &ConfigManager::getConfig()
{
	return config_;
}

bool ConfigManager::isLoaded() const
{
	return loaded_;
}

bool ConfigManager::createDefaultConfig()
{
	// This method creates a default config.json file with all default values
	// It is called automatically when no config file exists during load()
	// It can also be called explicitly to reset the config to defaults

	// Ensure config_ has default values
	config_ = Config::UserConfig{};

	// save() will handle creating the directory and writing the file
	return save();
}
