/**
 * @file llamaServerProcessWindows.cpp
 * @brief Windows-specific implementation for launching llama-server.
 *
 * Uses CreateProcessA() to spawn the llama-server process.
 * Uses CREATE_NO_WINDOW flag to prevent console window popup.
 */

#include "llamaServerProcess.h"
#include <iostream>
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
		// Build command arguments
		auto args = buildCommandArgs(modelPath, load, inference, server);

		// Log the command for debugging
		std::string logCmd = "llama-server";
		for (size_t i = 1; i < args.size(); ++i) {
			logCmd += " " + args[i];
		}
		std::cout << "Launching: " << logCmd << "\n";

		// Convert args to command line string with proper quoting
		std::string cmdLine = buildCommandLine(args);

		STARTUPINFOA si = {};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		PROCESS_INFORMATION pi = {};

		// Create the process with CREATE_NO_WINDOW to avoid console popup
		BOOL success = CreateProcessA(
			"llama-server",						 // lpApplicationName
			cmdLine.data(),						 // lpCommandLine
			nullptr,							 // lpProcessAttributes
			nullptr,							 // lpThreadAttributes
			FALSE,								 // bInheritHandles
			CREATE_NO_WINDOW | DETACHED_PROCESS, // dwCreationFlags
			nullptr,							 // lpEnvironment
			nullptr,							 // lpCurrentDirectory
			&si,								 // lpStartupInfo
			&pi									 // lpProcessInformation
		);

		if (!success) {
			DWORD error = GetLastError();
			std::cout << "CreateProcessA failed: " << error << "\n";
			return false;
		}

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