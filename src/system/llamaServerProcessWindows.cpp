/**
 * @file llamaServerProcessWindows.cpp
 * @brief Windows-specific implementation for launching llama-server.
 *
 * Uses CreateProcessA() to spawn the llama-server process.
 * Redirects stdout/stderr to a log file in .workbench/logs/
 */

#include "configManager.h"
#include "llamaServerProcess.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

namespace {
void logDebug(const std::string &msg)
{
	std::string logsDir = ConfigManager::getLogsDir();
	std::string debugLogPath = logsDir + "\\debug.log";
	std::ofstream debugFile(debugLogPath, std::ios::app);
	debugFile << msg << "\n";
}
} // namespace

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
		// Build command arguments
		auto args = buildCommandArgs(modelPath, load, inference, server);

		// Log the command for debugging
		std::string logCmd = "llama-server";
		for (size_t i = 1; i < args.size(); ++i) {
			logCmd += " " + args[i];
		}
		logDebug("=== launch called ===");
		logDebug("logCmd: " + logCmd);

		// Convert args to command line string with proper quoting
		std::string cmdLine = buildCommandLine(args);
		logDebug("cmdLine: " + cmdLine);

		// Get log path - redirect to .workbench/logs/llama-server.log
		std::string logsDir = ConfigManager::getLogsDir();
		std::string logPath = logsDir + "\\llama-server.log";
		logDebug("logsDir: " + logsDir);
		logDebug("logPath: " + logPath);

		// Create logs directory if it doesn't exist
		CreateDirectoryA(logsDir.c_str(), NULL);

		logDebug("About to launch with cmd /c");

		// Setup for stdout/stderr redirection to log file
		STARTUPINFOA si = {};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;

		// Create log file handles
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		HANDLE hLogFile = CreateFileA(logPath.c_str(),
									  GENERIC_WRITE,
									  FILE_SHARE_WRITE,
									  &sa,
									  CREATE_ALWAYS,
									  FILE_ATTRIBUTE_NORMAL,
									  NULL);

		if (hLogFile != INVALID_HANDLE_VALUE) {
			si.hStdOutput = hLogFile;
			si.hStdError = hLogFile;
		}

		PROCESS_INFORMATION pi = {};

		// Create the process - use cmd /c to search PATH
		// Build: "cmd /c llama-server <args>"
		std::string fullCmd = "cmd /c " + cmdLine;
		logDebug("fullCmd: " + fullCmd);
		BOOL success = CreateProcessA(
			NULL,								 // lpApplicationName
			fullCmd.data(),						 // lpCommandLine
			nullptr,							 // lpProcessAttributes
			nullptr,							 // lpThreadAttributes
			FALSE,								 // bInheritHandles
			CREATE_NO_WINDOW | DETACHED_PROCESS, // dwCreationFlags
			nullptr,							 // lpEnvironment
			nullptr,							 // lpCurrentDirectory
			&si,								 // lpStartupInfo
			&pi									 // lpProcessInformation
		);

		// Close log file handle
		if (hLogFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hLogFile);
		}

		if (!success) {
			DWORD error = GetLastError();
			logDebug("CreateProcessA FAILED, error: " + std::to_string(error));
			return false;
		}

		logDebug("CreateProcessA SUCCESS");
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

		// Try graceful termination first using GenerateConsoleCtrlEvent
		// This only works if the process is in the same console
		// For DETACHED_PROCESS, we need to use TerminateProcess
		BOOL result = TerminateProcess(processHandle_, 1);
		if (result) {
			WaitForSingleObject(processHandle_, INFINITE);
		}

		CloseHandle(processHandle_);
		processHandle_ = nullptr;
		running_ = false;
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
			// Process has exited
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
			// Quote arguments that contain spaces
			if (arg.find(' ') != std::string::npos) {
				cmdLine += "\"" + arg + "\" ";
			} else {
				cmdLine += arg + " ";
			}
		}
		// Remove trailing space
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

LlamaServerProcess &LlamaServerProcess::instance()
{
	static LlamaServerProcess process;
	return process;
}

std::string LlamaServerProcess::getLogPath()
{
	return ConfigManager::getLogsDir() + "/llama-server.log";
}