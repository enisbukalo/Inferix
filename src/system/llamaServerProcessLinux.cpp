/**
 * @file llamaServerProcessLinux.cpp
 * @brief Linux-specific implementation for launching llama-server.
 *
 * Uses fork() + execve() to spawn the llama-server process.
 * Redirects stdout/stderr to /dev/null to run as a background process.
 */

#include "llamaServerProcess.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
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

		// Log the command for debugging
		std::string logCmd = "llama-server";
		for (size_t i = 1; i < args.size(); ++i) {
			logCmd += " " + args[i];
		}
		std::cout << "Launching: " << logCmd << "\n";

		// Fork the process
		pid_ = fork();
		if (pid_ < 0) {
			std::cout << "fork() failed: " << strerror(errno) << "\n";
			return false;
		}

		if (pid_ == 0) {
			// Child process
			launchChild(args);
			// If launchChild returns, execve failed
			_exit(1);
		}

		// Parent process - check if child started successfully
		running_ = true;
		return true;
	}

	bool terminate()
	{
		if (!running_ || pid_ < 0) {
			return false;
		}

		// Try graceful termination first with SIGTERM
		if (kill(pid_, SIGTERM) == 0) {
			// Wait up to 5 seconds for graceful shutdown
			for (int i = 0; i < 50; ++i) {
				int status;
				pid_t result = waitpid(pid_, &status, WNOHANG);
				if (result == pid_) {
					running_ = false;
					pid_ = -1;
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
		// Redirect stdout and stderr to /dev/null
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, STDOUT_FILENO);
			dup2(devnull, STDERR_FILENO);
			close(devnull);
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