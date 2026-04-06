/**
 * @file llamaServerProcessWindows.cpp
 * @brief Windows-specific implementation for launching llama-server.
 *
 * Uses CreateProcessA() to spawn the llama-server process.
 * Logging is handled by llama-server via --log-file flag.
 */

#include "configManager.h"
#include "llamaServerProcess.h"

#include <chrono>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

class LlamaServerProcess::Impl
{
  public:
	Impl() : processHandle_(nullptr), running_(false), httpClient_()
	{
	}

	~Impl()
	{
		if (running_) {
			terminate();
		}
	}

	bool launch(const std::string &modelPath,
				const Config::LoadSettings &load,
				const Config::InferenceSettings &inference,
				const Config::ServerSettings &server)
	{
		// Build command arguments (includes --log-file)
		auto args = buildCommandArgs(modelPath, load, inference, server);

		spdlog::debug("Building llama-server command");

		spdlog::info("Starting llama-server with model: '{}'", modelPath);

		// Build full command for logging
		std::string exePath =
			"C:\\Users\\bukal\\Documents\\llama\\llama-server.exe";
		std::string fullCommand = exePath + " " + buildCommandLine(args);
		spdlog::info("llama-server command: {}", fullCommand);

		// Setup for hidden window
		STARTUPINFOA si = {};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		PROCESS_INFORMATION pi = {};

		// Use full path to llama-server executable
		std::string argsOnly = buildCommandLine(args);

		BOOL success = CreateProcessA(exePath.c_str(),	// lpApplicationName
									  argsOnly.data(),	// lpCommandLine
									  nullptr,			// lpProcessAttributes
									  nullptr,			// lpThreadAttributes
									  FALSE,			// bInheritHandles
									  CREATE_NO_WINDOW, // dwCreationFlags
									  nullptr,			// lpEnvironment
									  nullptr,			// lpCurrentDirectory
									  &si,				// lpStartupInfo
									  &pi				// lpProcessInformation
		);

		if (!success) {
			DWORD error = GetLastError();
			spdlog::error(
				"Failed to start llama-server: CreateProcessA error {}",
				error);
			return false;
		}

		spdlog::info("llama-server started (PID: {})", pi.dwProcessId);

		// Close thread handle - we only need process handle
		CloseHandle(pi.hThread);

		processHandle_ = pi.hProcess;
		running_ = true;
		return true;
	}

	bool terminate()
	{
		if (!running_ || processHandle_ == nullptr) {
			return false;
		}

		spdlog::info("Terminating llama-server...");

		BOOL result = TerminateProcess(processHandle_, 1);
		if (result) {
			WaitForSingleObject(processHandle_, INFINITE);
		}

		CloseHandle(processHandle_);
		processHandle_ = nullptr;
		running_ = false;

		if (result) {
			spdlog::info("llama-server terminated");
		}

		return result != FALSE;
	}

	bool isRunning() const
	{
		if (!running_ || processHandle_ == nullptr) {
			return false;
		}

		DWORD exitCode;
		if (GetExitCodeProcess(processHandle_, &exitCode)) {
			if (exitCode == STILL_ACTIVE) {
				return true;
			}
			return false;
		}
		return false;
	}

	intptr_t getHandle() const
	{
		return reinterpret_cast<intptr_t>(processHandle_);
	}

	bool isServerHealthy()
	{
		if (!running_)
			return false;
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/health";
		auto [success, response] = httpClient_.get(url);
		return success && response.find("ok") != std::string::npos;
	}

	bool isModelLoaded()
	{
		if (!isServerHealthy())
			return false;
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/slots";
		auto [success, response] = httpClient_.get(url);
		if (!success)
			return false;
		// Response contains "slots" array - if not empty, model is loaded
		return response.find("\"slots\":[]") == std::string::npos;
	}

	std::string getLoadedModelPath()
	{
		if (!isServerHealthy())
			return "";
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/slots";
		auto [success, response] = httpClient_.get(url);
		if (!success)
			return "";
		// Simple parsing: look for "model" field in JSON
		auto modelPos = response.find("\"model\"");
		if (modelPos == std::string::npos)
			return "";
		auto colonPos = response.find(":", modelPos);
		if (colonPos == std::string::npos)
			return "";
		// Find start of value (skip whitespace and quotes)
		size_t pos = colonPos + 1;
		while (pos < response.size() &&
			   (response[pos] == ' ' || response[pos] == '"'))
			pos++;
		size_t end = pos;
		while (end < response.size() && response[end] != '"' &&
			   response[end] != ',')
			end++;
		if (end > pos) {
			return response.substr(pos, end - pos);
		}
		return "";
	}

	bool unloadModel()
	{
		if (!isServerHealthy())
			return false;
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/props";
		std::string body = "{\"model\":\"\"}";
		auto [success, response] = httpClient_.post(url, body);
		if (success) {
			spdlog::info("Model unloaded successfully");
		} else {
			spdlog::error("Failed to unload model: {}", response);
		}
		return success;
	}

	bool loadModel(const std::string &modelPath)
	{
		if (!isServerHealthy())
			return false;
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/props";
		std::string body = "{\"model\":\"" + modelPath + "\"}";
		auto [success, response] = httpClient_.post(url, body);
		if (success) {
			spdlog::info("Model loaded: {}", modelPath);
		} else {
			spdlog::error("Failed to load model: {}", response);
		}
		return success;
	}

  private:
	std::string buildCommandLine(const std::vector<std::string> &args)
	{
		std::string cmdLine;
		for (const auto &arg : args) {
			if (arg.find(' ') != std::string::npos) {
				cmdLine += "\"" + arg + "\" ";
			} else {
				cmdLine += arg + " ";
			}
		}
		if (!cmdLine.empty() && cmdLine.back() == ' ') {
			cmdLine.pop_back();
		}
		return cmdLine;
	}

	HANDLE processHandle_;
	bool running_;
	HttpClient httpClient_;
};

LlamaServerProcess::LlamaServerProcess() : m_impl(std::make_unique<Impl>())
{
}

LlamaServerProcess::~LlamaServerProcess() = default;

bool LlamaServerProcess::launch(const std::string &modelPath,
								const Config::LoadSettings &load,
								const Config::InferenceSettings &inference,
								const Config::ServerSettings &server)
{
	return m_impl->launch(modelPath, load, inference, server);
}

bool LlamaServerProcess::terminate()
{
	return m_impl->terminate();
}

bool LlamaServerProcess::isRunning() const
{
	return m_impl->isRunning();
}

intptr_t LlamaServerProcess::getHandle() const
{
	return m_impl->getHandle();
}

void LlamaServerProcess::setOutputCallback(
	std::function<void(const std::string &)> callback)
{
	// Not used - output goes to log file via --log-file
	(void)callback;
}

LlamaServerProcess &LlamaServerProcess::instance()
{
	static LlamaServerProcess process;
	return process;
}

std::string LlamaServerProcess::getLogPath()
{
	return ConfigManager::getLogsDir() + "/llama-server.log";
}

bool LlamaServerProcess::isModelLoaded()
{
	return m_impl->isModelLoaded();
}

std::string LlamaServerProcess::getLoadedModelPath()
{
	return m_impl->getLoadedModelPath();
}

bool LlamaServerProcess::unloadModel()
{
	return m_impl->unloadModel();
}

bool LlamaServerProcess::loadModel(const std::string &modelPath)
{
	return m_impl->loadModel(modelPath);
}

bool LlamaServerProcess::isServerHealthy()
{
	return m_impl->isServerHealthy();
}