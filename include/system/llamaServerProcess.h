#pragma once

#include "config.h"
#include "configManager.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @class LlamaServerProcess
 * @brief Platform-agnostic interface for launching and managing llama-server.
 *
 * This class uses the pImpl idiom - the actual implementation is in the
 * platform- specific .cpp files (llamaServerProcessLinux.cpp,
 * llamaServerProcessWindows.cpp).
 *
 * Usage:
 *   auto process = std::make_unique<LlamaServerProcess>();
 *   bool success = process->launch(modelPath, loadSettings, inferenceSettings,
 * serverSettings);
 *   // ...
 *   process->terminate();  // On app exit
 *
 * Or use the singleton for global cleanup:
 *   LlamaServerProcess::instance().terminate();
 */
class LlamaServerProcess
{
  public:
	/**
	 * @brief Get the singleton instance.
	 *
	 * Provides global access to the process handle for cleanup during
	 * application shutdown. Uses Meyers' singleton pattern.
	 *
	 * @return Reference to the singleton instance.
	 */
	static LlamaServerProcess &instance();

	LlamaServerProcess();
	~LlamaServerProcess();

	// Delete copy/move - process handle is not copyable
	LlamaServerProcess(const LlamaServerProcess &) = delete;
	LlamaServerProcess &operator=(const LlamaServerProcess &) = delete;
	LlamaServerProcess(LlamaServerProcess &&) = delete;
	LlamaServerProcess &operator=(LlamaServerProcess &&) = delete;

	/**
	 * @brief Launch llama-server with the given configuration.
	 * @param modelPath Full path to the .gguf model file
	 * @param load Load settings (GPU layers, context size, etc.)
	 * @param inference Inference settings (temperature, sampling, etc.)
	 * @param server Server settings (host, port, etc.)
	 * @return true if launch succeeded, false on failure
	 */
	bool launch(const std::string &modelPath,
				const Config::LoadSettings &load,
				const Config::InferenceSettings &inference,
				const Config::ServerSettings &server);

	/**
	 * @brief Terminate the running llama-server process gracefully.
	 * @return true if terminated successfully, false on failure
	 */
	bool terminate();

	/**
	 * @brief Check if the process is currently running.
	 * @return true if running, false otherwise
	 */
	bool isRunning() const;

	/**
	 * @brief Get the platform-specific process handle.
	 * @return On Linux: pid_t (process ID)
	 *         On Windows: HANDLE (process handle)
	 */
	intptr_t getHandle() const;

	/**
	 * @brief Build command argument vector from settings.
	 * @param modelPath Path to the model file
	 * @param load Load settings
	 * @param inference Inference settings
	 * @param server Server settings
	 * @return Vector of command arguments suitable for execve() or conversion
	 */
	static std::vector<std::string>
	buildCommandArgs(const std::string &modelPath,
					 const Config::LoadSettings &load,
					 const Config::InferenceSettings &inference,
					 const Config::ServerSettings &server);

	/**
	 * @brief Get the path to the llama-server log file.
	 * @return The full path to the log file (e.g.,
	 * ~/.workbench/logs/llama-server.log)
	 */
	static std::string getLogPath();

  private:
	class Impl; // Forward declaration for pImpl
	std::unique_ptr<Impl> m_impl;
};