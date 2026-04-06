#include "modelsIni.h"
#include "configManager.h"

#include <fstream>
#include <spdlog/spdlog.h>
#include <sstream>

namespace {
/**
 * @brief Trim whitespace from string
 */
std::string trim(const std::string &str)
{
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

/**
 * @brief Check if line is a comment or empty
 */
bool isCommentOrEmpty(const std::string &line)
{
	std::string trimmed = trim(line);
	return trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#';
}
} // namespace

ModelsIni &ModelsIni::instance()
{
	static ModelsIni instance;
	return instance;
}

std::string ModelsIni::getPath() const
{
	return m_filePath;
}

bool ModelsIni::load()
{
	std::string configDir = ConfigManager::instance().getConfigDir();
	m_filePath = configDir + "/models.ini";
	spdlog::debug("Loading models.ini from: {}", m_filePath);

	std::ifstream file(m_filePath);
	if (!file.is_open()) {
		spdlog::warn("models.ini not found, will create default");
		return false;
	}

	m_sections.clear();
	std::string currentSection;
	std::string line;

	while (std::getline(file, line)) {
		// Skip comments and empty lines
		if (isCommentOrEmpty(line))
			continue;

		// Check for section header: [section_name]
		if (line[0] == '[') {
			size_t end = line.find(']');
			if (end != std::string::npos) {
				currentSection = trim(line.substr(1, end - 1));
				// Create empty section if it doesn't exist
				if (m_sections.find(currentSection) == m_sections.end()) {
					m_sections[currentSection] = {};
				}
			}
			continue;
		}

		// Parse key = value
		size_t equalsPos = line.find('=');
		if (equalsPos != std::string::npos && !currentSection.empty()) {
			std::string key = trim(line.substr(0, equalsPos));
			std::string value = trim(line.substr(equalsPos + 1));
			m_sections[currentSection][key] = value;
		}
	}

	spdlog::info("Loaded {} model(s) from models.ini", getModelNames().size());
	return true;
}

bool ModelsIni::createDefault()
{
	std::string configDir = ConfigManager::instance().getConfigDir();
	m_filePath = configDir + "/models.ini";

	std::ofstream file(m_filePath);
	if (!file.is_open()) {
		spdlog::error("Failed to create models.ini at: {}", m_filePath);
		return false;
	}

	file << "version = 1\n";
	file << "\n";
	file << "; Global defaults for all models\n";
	file << "[*]\n";
	file << "; Add global defaults here, e.g.:\n";
	file << "; c = 4096\n";
	file << "; n-gpu-layers = 99\n";
	file << "\n";
	file << "; Example model entries:\n";
	file << ";\n";
	file << "; [orchestrator]\n";
	file << "; model = "
			"D:/Models/bartowski/nvidia_Orchestrator-8B-GGUF/"
			"nvidia_Orchestrator-8B-Q6_K_L.gguf\n";
	file << ";\n";
	file << "; [qwen-27b]\n";
	file << "; model = "
			"D:/Models/bartowski/Qwen_Qwen3.5-27B-GGUF/"
			"Qwen_Qwen3.5-27B-Q4_K_M.gguf\n";

	file.close();

	spdlog::info("Created default models.ini at: {}", m_filePath);

	// Also load the newly created file
	return load();
}

std::vector<std::string> ModelsIni::getModelNames() const
{
	std::vector<std::string> names;
	for (const auto &section : m_sections) {
		// Skip the global [*] section
		if (section.first != "*") {
			names.push_back(section.first);
		}
	}
	return names;
}

std::string ModelsIni::getModelPath(const std::string &sectionName) const
{
	auto sectionIt = m_sections.find(sectionName);
	if (sectionIt == m_sections.end()) {
		return "";
	}

	auto pathIt = sectionIt->second.find("model");
	if (pathIt == sectionIt->second.end()) {
		return "";
	}

	return pathIt->second;
}

std::map<std::string, std::string>
ModelsIni::getSectionValues(const std::string &sectionName) const
{
	auto sectionIt = m_sections.find(sectionName);
	if (sectionIt == m_sections.end()) {
		return {};
	}
	return sectionIt->second;
}

std::map<std::string, std::string> ModelsIni::getGlobalDefaults() const
{
	auto globalIt = m_sections.find("*");
	if (globalIt == m_sections.end()) {
		return {};
	}
	return globalIt->second;
}