#pragma once

#include <string>
#include <vector>
#include <map>

/**
 * @brief ModelsIni - Handler for llama-server models.ini preset file.
 *
 * Loads and parses the models.ini file from the config directory.
 * Each section (except [*] for global defaults) represents a model.
 *
 * File format:
 *   version = 1
 *   [*]
 *   ; global defaults
 *   c = 4096
 *
 *   [orchestrator]
 *   model = D:/Models/bartowski/nvidia_Orchestrator-8B-GGUF/nvidia_Orchestrator-8B-Q6_K_L.gguf
 */
class ModelsIni
{
  public:
	/**
	 * @brief Get singleton instance
	 */
	static ModelsIni &instance();

	/**
	 * @brief Load the models.ini file from config directory
	 * @return true if loaded successfully, false if file doesn't exist
	 */
	bool load();

	/**
	 * @brief Create a default models.ini file if it doesn't exist
	 * @return true if created successfully
	 */
	bool createDefault();

	/**
	 * @brief Get the path to the models.ini file
	 */
	std::string getPath() const;

	/**
	 * @brief Get all model section names (excluding global [*])
	 * @return Vector of section names
	 */
	std::vector<std::string> getModelNames() const;

	/**
	 * @brief Get the model path for a given section name
	 * @param sectionName The section name (e.g., "orchestrator")
	 * @return The model path, or empty if not found
	 */
	std::string getModelPath(const std::string &sectionName) const;

	/**
	 * @brief Get all key-value pairs for a section
	 * @param sectionName The section name
	 * @return Map of key-value pairs
	 */
	std::map<std::string, std::string>
	getSectionValues(const std::string &sectionName) const;

	/**
	 * @brief Get global default values from [*] section
	 * @return Map of key-value pairs
	 */
	std::map<std::string, std::string> getGlobalDefaults() const;

  private:
	ModelsIni() = default;
	~ModelsIni() = default;
	ModelsIni(const ModelsIni &) = delete;
	ModelsIni &operator=(const ModelsIni &) = delete;

	std::string m_filePath;
	std::map<std::string, std::map<std::string, std::string>> m_sections;
};