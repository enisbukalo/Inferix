#pragma once
#include "models/config/serverSettings.h"
#include "models/config/modelSettings.h"
#include "models/config/uiSettings.h"
#include "models/config/userSettings.h"
#include <optional>
#include <vector>
#include "json.hpp"

/**
 * @file config.h
 * @brief UserConfig aggregate struct and JSON serialization declarations.
 *
 * This header serves as the central aggregation point for all configuration
 * structures in the Workbench application. It includes the domain-specific
 * config headers and declares the to_json/from_json functions required for
 * nlohmann::json serialization.
 *
 * The UserConfig struct is the root configuration object that gets serialized
 * to the config file (~/.workbench/config.json). Its structure mirrors the
 * JSON format:
 * @code
 * {
 *   "server": { ... },      // ServerSettings
 *   "load": { ... },        // LoadSettings
 *   "inference": { ... },   // InferenceSettings
 *   "ui": { ... },          // UISettings
 *   "terminal": { ... },    // TerminalSettings
 *   "presets": [ ... ],     // vector<ModelPreset>
 *   "terminalPresets": [ .. ] // vector<TerminalPreset>
 * }
 * @endcode
 *
 * @see ConfigManager for loading and saving configuration
 * @see serverSettings.h for ServerSettings documentation
 * @see modelSettings.h for LoadSettings and InferenceSettings documentation
 * @see uiSettings.h for UISettings documentation
 * @see userSettings.h for TerminalSettings documentation
 */
namespace Config {

/**
 * @brief Main configuration container.
 *
 * Aggregates all configuration categories into a single structure.
 * This is the root object that gets serialized to/from the JSON
 * config file. Each member corresponds to a top-level key in the JSON.
 *
 * This struct uses aggregate initialization, allowing initialization
 * with brace syntax: `UserConfig config{...};`
 *
 * @note All members have default values defined in their respective
 *       struct definitions, so `UserConfig config;` creates a valid
 *       configuration with sensible defaults.
 * @note The struct is designed to be trivially copyable and movable.
 *
 * @code
 * // Create with defaults
 * UserConfig config;
 *
 * // Create with custom values
 * UserConfig config{
 *     ServerSettings{.host = "0.0.0.0", .port = 8080},
 *     LoadSettings{.modelPath = "models/mistral.gguf"},
 *     InferenceSettings{.temperature = 0.7f},
 *     UISettings{},
 *     TerminalSettings{},
 *     {},
 *     {}
 * };
 * @endcode
 */
struct UserConfig
{
	/**
	 * @brief Server and network configuration.
	 *
	 * Contains settings for the HTTP server including host, port,
	 * API keys, SSL certificates, and server behavior options.
	 * These settings map directly to llama-server CLI parameters.
	 *
	 * @see serverSettings.h for detailed field documentation
	 */
	ServerSettings server;

	/**
	 * @brief Model loading and memory configuration.
	 *
	 * Contains settings for model source (path, URL, HuggingFace),
	 * GPU configuration, context size, batching, KV cache,
	 * and memory management options.
	 *
	 * @see modelSettings.h for detailed field documentation
	 */
	LoadSettings load;

	/**
	 * @brief Inference and sampling configuration.
	 *
	 * Contains settings for text generation including temperature,
	 * top-k/top-p sampling, penalties, grammar constraints, and
	 * other sampling parameters.
	 *
	 * @see modelSettings.h for detailed field documentation
	 */
	InferenceSettings inference;

	/**
	 * @brief User interface configuration.
	 *
	 * Contains settings for the terminal UI including theme,
	 * default tab, system panel visibility, and refresh rate.
	 *
	 * @see uiSettings.h for detailed field documentation
	 */
	UISettings ui;

	/**
	 * @brief Terminal emulator configuration.
	 *
	 * Contains settings for the embedded terminal including
	 * default shell, initial command, working directory, and
	 * default dimensions.
	 *
	 * @see userSettings.h for detailed field documentation
	 */
	TerminalSettings terminal;

	/**
	 * @brief Named model presets.
	 *
	 * A list of named presets that bundle a model selection with
	 * its associated inference settings. Allows users to quickly
	 * switch between different model configurations.
	 *
	 * @see modelSettings.h for ModelPreset documentation
	 */
	std::vector<ModelPreset> presets;

	/**
	 * @brief Named terminal presets.
	 *
	 * A list of named terminal configurations that appear as
	 * dynamic top-level tabs in the application. Each preset
	 * defines a command to run and optional dimensions.
	 *
	 * @see userSettings.h for TerminalPreset documentation
	 */
	std::vector<TerminalPreset> terminalPresets;
};

/**
 * @name JSON Serialization Functions
 * @{
 *
 * These functions enable nlohmann::json to automatically serialize
 * and deserialize configuration structures. They follow the naming
 * convention required by the nlohmann::json library for custom types.
 *
 * The serialization is recursive - to_json for UserConfig calls
 * to_json for each member, which in turn may call to_json for
 * nested structures.
 *
 * @note These functions are declared here but defined in config.cpp.
 * @note The from_json functions use j.value(key, default) to provide
 *       graceful fallback to default values when keys are missing.
 */

/** @brief Serialize ServerSettings to JSON. */
void to_json(nlohmann::json &j, const ServerSettings &v);
/** @brief Deserialize ServerSettings from JSON. */
void from_json(const nlohmann::json &j, ServerSettings &v);

/** @brief Serialize LoadSettings to JSON. */
void to_json(nlohmann::json &j, const LoadSettings &v);
/** @brief Deserialize LoadSettings from JSON. */
void from_json(const nlohmann::json &j, LoadSettings &v);

/** @brief Serialize InferenceSettings to JSON. */
void to_json(nlohmann::json &j, const InferenceSettings &v);
/** @brief Deserialize InferenceSettings from JSON. */
void from_json(const nlohmann::json &j, InferenceSettings &v);

/** @brief Serialize UISettings to JSON. */
void to_json(nlohmann::json &j, const UISettings &v);
/** @brief Deserialize UISettings from JSON. */
void from_json(const nlohmann::json &j, UISettings &v);

/** @brief Serialize TerminalSettings to JSON. */
void to_json(nlohmann::json &j, const TerminalSettings &v);
/** @brief Deserialize TerminalSettings from JSON. */
void from_json(const nlohmann::json &j, TerminalSettings &v);

/** @brief Serialize ModelPreset to JSON. */
void to_json(nlohmann::json &j, const ModelPreset &v);
/** @brief Deserialize ModelPreset from JSON. */
void from_json(const nlohmann::json &j, ModelPreset &v);

/** @brief Serialize TerminalPreset to JSON. */
void to_json(nlohmann::json &j, const TerminalPreset &v);
/** @brief Deserialize TerminalPreset from JSON. */
void from_json(const nlohmann::json &j, TerminalPreset &v);

/** @brief Serialize UserConfig to JSON. */
void to_json(nlohmann::json &j, const UserConfig &v);
/** @brief Deserialize UserConfig from JSON. */
void from_json(const nlohmann::json &j, UserConfig &v);

/** @} */

} // namespace Config
