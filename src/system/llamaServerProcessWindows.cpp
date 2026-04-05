/**
 * @file llamaServerProcessWindows.cpp
 * @brief Windows-specific implementation for launching llama-server.
 *
 * Uses CreateProcessA() to spawn the llama-server process.
 * Captures stdout/stderr via anonymous pipes and forwards to a callback.
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

	void setOutputCallback(std::function<void(const std::string &)> callback)
	{
		outputCallback_ = callback;
	}

	bool launch(const std::string &modelPath,
				const Config::LoadSettings &load,
				const Config::InferenceSettings &inference,
				const Config::ServerSettings &server)
	{
		// Build command arguments
		auto args = buildCommandArgs(modelPath, load, inference, server);

		spdlog::debug("Building llama-server command");

		spdlog::info("Starting llama-server with model: '{}'", modelPath);

		// Setup for stdout/stderr redirection using pipes
		STARTUPINFOA si = {};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;

		// Create pipes for stdout and stderr
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

		HANDLE hStdoutRead, hStdoutWrite;
		HANDLE hStderrRead, hStderrWrite;

		if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0)) {
			spdlog::error("Failed to create stdout pipe");
			return false;
		}

		if (!CreatePipe(&hStderrRead, &hStderrWrite, &sa, 0)) {
			CloseHandle(hStdoutRead);
			CloseHandle(hStdoutWrite);
			spdlog::error("Failed to create stderr pipe");
			return false;
		}

		// Set the write ends as stdout/stderr for the child
		si.hStdOutput = hStdoutWrite;
		si.hStdError = hStderrWrite;

		PROCESS_INFORMATION pi = {};

		// Use full path to llama-server executable
		std::string exePath =
			"C:\\Users\\bukal\\Documents\\llama\\llama-server.exe";
		std::string argsOnly = buildCommandLine(args);

		BOOL success =
			CreateProcessA(exePath.c_str(),	 // lpApplicationName - full path
						   argsOnly.data(),	 // lpCommandLine - just args
						   nullptr,			 // lpProcessAttributes
						   nullptr,			 // lpThreadAttributes
						   FALSE,			 // bInheritHandles
						   CREATE_NO_WINDOW, // dwCreationFlags - hide window
						   nullptr,			 // lpEnvironment
						   nullptr,			 // lpCurrentDirectory
						   &si,				 // lpStartupInfo
						   &pi				 // lpProcessInformation
			);

		// Close the write ends in parent - child has copies
		CloseHandle(hStdoutWrite);
		CloseHandle(hStderrWrite);

		if (!success) {
			DWORD error = GetLastError();
			CloseHandle(hStdoutRead);
			CloseHandle(hStderrRead);
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

		// Start background thread to read from pipes
		stopPipeReader_.store(false);
		pipeReadThread_ = std::thread([this, hStdoutRead, hStderrRead]() {
			readPipe(hStdoutRead);
			readPipe(hStderrRead);
		});

		return true;
	}

	bool terminate()
	{
		if (!running_ || processHandle_ == nullptr) {
			return false;
		}

		spdlog::info("Terminating llama-server...");

		// Stop the pipe reader first
		stopPipeReader_.store(true);
		if (pipeReadThread_.joinable()) {
			pipeReadThread_.join();
		}

		// Then terminate the process
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
	void readPipe(HANDLE hRead)
	{
		const size_t bufferSize = 4096;
		char buffer[bufferSize];
		DWORD bytesRead;
		std::string lineBuffer;

		while (!stopPipeReader_.load()) {
			BOOL result =
				ReadFile(hRead, buffer, bufferSize - 1, &bytesRead, NULL);
			if (!result || bytesRead == 0) {
				break;
			}

			buffer[bytesRead] = '\0';

			// Process character by character to handle line breaks
			for (DWORD i = 0; i < bytesRead; ++i) {
				if (buffer[i] == '\n' || buffer[i] == '\r') {
					if (!lineBuffer.empty()) {
						if (outputCallback_) {
							outputCallback_(lineBuffer);
						}
						lineBuffer.clear();
					}
				} else {
					lineBuffer += buffer[i];
				}
			}
		}

		// Send any remaining data
		if (!lineBuffer.empty() && outputCallback_) {
			outputCallback_(lineBuffer);
		}

		CloseHandle(hRead);
	}

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
	std::function<void(const std::string &)> outputCallback_;
	std::thread pipeReadThread_;
	std::atomic<bool> stopPipeReader_{ false };
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
	m_impl->setOutputCallback(callback);
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