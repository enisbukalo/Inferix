/**
 * @file llamaServerProcessWindows.cpp
 * @brief Windows-specific implementation for launching llama-server.
 *
 * Uses CreateProcessA() to spawn the llama-server process.
 * Logging is handled by llama-server via --log-file flag.
 */

#include "configManager.h"
#include "llamaServerProcess.h"

#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <windows.h>

class LlamaServerProcess::Impl
{
  public:
	Impl() : processHandle_(nullptr), running_(false)
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

		// Setup for hidden window
		STARTUPINFOA si = {};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		PROCESS_INFORMATION pi = {};

		// Use full path to llama-server executable
		std::string exePath =
			"C:\\Users\\bukal\\Documents\\llama\\llama-server.exe";
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