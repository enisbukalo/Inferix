/**
 * @file llamaServerProcessLinux.cpp
 * @brief Linux-specific implementation for launching llama-server.
 *
 * Uses fork() + execve() to spawn the llama-server process.
 * Redirects stdout/stderr to a log file in .workbench/logs/
 */

#include "configManager.h"
#include "llamaServerProcess.h"

#include <spdlog/spdlog.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

class LlamaServerProcess::Impl
{
  public:
	Impl() : pid_(-1), running_(false)
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

		spdlog::debug("Building llama-server command");

		// Get log path - redirect to .workbench/logs/llama-server.log
		std::string logsDir = ConfigManager::getLogsDir();
		std::string logPath = logsDir + "/llama-server.log";

		// Create logs directory if it doesn't exist
		std::filesystem::create_directories(logsDir);

		spdlog::info("Starting llama-server with model: '{}'", modelPath);

		// Fork the process
		pid_ = fork();
		if (pid_ < 0) {
			spdlog::error("Failed to start llama-server: fork() failed: {}", strerror(errno));
			return false;
		}

		if (pid_ == 0) {
			// Child process
			launchChild(args);
			// If launchChild returns, execve failed
			_exit(1);
		}

		// Parent process - check if child started successfully
		spdlog::info("llama-server started (PID: {})", pid_);
		running_ = true;
		return true;
	}

	bool terminate()
	{
		if (!running_ || pid_ < 0) {
			return false;
		}

		spdlog::info("Terminating llama-server...");

		// Try graceful termination first with SIGTERM
		if (kill(pid_, SIGTERM) == 0) {
			// Wait up to 5 seconds for graceful shutdown
			for (int i = 0; i < 50; ++i) {
				int status;
				pid_t result = waitpid(pid_, &status, WNOHANG);
				if (result == pid_) {
					running_ = false;
					pid_ = -1;
					spdlog::info("llama-server terminated");
					return true;
				}
				usleep(100000); // 100ms
			}
		}

		// Force kill if still running
		if (kill(pid_, SIGKILL) == 0) {
			waitpid(pid_, nullptr, 0);
		}

		running_ = false;
		pid_ = -1;
		spdlog::info("llama-server terminated");
		return true;
	}

	bool isRunning() const
	{
		if (!running_ || pid_ < 0) {
			return false;
		}
		int status;
		pid_t result = waitpid(pid_, &status, WNOHANG);
		if (result == pid_) {
			// Process has exited
			return false;
		}
		return true;
	}

	intptr_t getHandle() const
	{
		return static_cast<intptr_t>(pid_);
	}

  private:
	void launchChild(const std::vector<std::string> &args)
	{
		// Get log path - redirect to .workbench/logs/llama-server.log
		std::string logsDir = ConfigManager::getLogsDir();
		std::string logPath = logsDir + "/llama-server.log";

		// Create logs directory if it doesn't exist
		std::filesystem::create_directories(logsDir);

		// Open log file
		int logfd = open(logPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (logfd >= 0) {
			dup2(logfd, STDOUT_FILENO);
			dup2(logfd, STDERR_FILENO);
			close(logfd);
		}

		// Convert std::vector<std::string> to char* const* for execve
		std::vector<char *> argv;
		for (const auto &arg : args) {
			argv.push_back(const_cast<char *>(arg.c_str()));
		}
		argv.push_back(nullptr);

		// Execute llama-server
		execve("llama-server", argv.data(), environ);
		// If we get here, execve failed
	}

	pid_t pid_;
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