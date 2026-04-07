/**
 * @file llamaServerProcessLinux.cpp
 * @brief Linux-specific implementation for launching llama-server.
 *
 * Uses fork() + execve() to spawn the llama-server process.
 * Captures stdout/stderr via pipes and forwards to a callback.
 */

#include "configManager.h"
#include "llamaServerProcess.h"

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <json.hpp>
#include <signal.h>
#include <spdlog/spdlog.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

class LlamaServerProcess::Impl
{
  public:
	Impl() : pid_(-1), running_(false), httpClient_()
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

		// Create pipes for stdout and stderr
		int stdoutPipe[2];
		int stderrPipe[2];

		if (pipe(stdoutPipe) < 0) {
			spdlog::error("Failed to create stdout pipe");
			return false;
		}

		if (pipe(stderrPipe) < 0) {
			close(stdoutPipe[0]);
			close(stdoutPipe[1]);
			spdlog::error("Failed to create stderr pipe");
			return false;
		}

		// Fork the process
		pid_ = fork();
		if (pid_ < 0) {
			close(stdoutPipe[0]);
			close(stdoutPipe[1]);
			close(stderrPipe[0]);
			close(stderrPipe[1]);
			spdlog::error("Failed to start llama-server: fork() failed: {}",
						  strerror(errno));
			return false;
		}

		if (pid_ == 0) {
			// Child process - redirect stdout/stderr and exec
			close(stdoutPipe[0]); // Close read end
			close(stderrPipe[0]); // Close read end

			dup2(stdoutPipe[1], STDOUT_FILENO);
			dup2(stderrPipe[1], STDERR_FILENO);
			close(stdoutPipe[1]);
			close(stderrPipe[1]);

			// Ask the kernel to send SIGKILL to this child when the parent dies
			prctl(PR_SET_PDEATHSIG, SIGKILL);

			// Detach from controlling terminal
			setsid();

			// Convert args to char* array
			std::vector<char *> argv;
			for (const auto &arg : args) {
				argv.push_back(const_cast<char *>(arg.c_str()));
			}
			argv.push_back(nullptr);

			execve(server.executablePath.empty() ? "llama-server"
												 : server.executablePath.c_str(),
				   argv.data(),
				   environ);
			// If we get here, execve failed
			_exit(1);
		}

		// Parent process - close write ends and start reading
		close(stdoutPipe[1]);
		close(stderrPipe[1]);

		spdlog::info("llama-server started (PID: {})", pid_);
		running_ = true;

		// Start background thread to read from pipes
		// Copy file descriptors to avoid capture issues with arrays
		int stdoutFd = stdoutPipe[0];
		int stderrFd = stderrPipe[0];
		stopPipeReader_.store(false);
		pipeReadThread_ = std::thread([this, stdoutFd, stderrFd]() {
			readPipe(stdoutFd);
			readPipe(stderrFd);
		});

		return true;
	}

	bool terminate()
	{
		if (!running_ || pid_ < 0) {
			return false;
		}

		spdlog::info("Terminating llama-server...");

		// Stop the pipe reader first
		stopPipeReader_.store(true);
		if (pipeReadThread_.joinable()) {
			pipeReadThread_.join();
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
						  std::to_string(cfg.server.port) + "/models";
		auto [success, response] = httpClient_.get(url);
		if (!success) {
			spdlog::debug("isModelLoaded failed: {}", response);
			return false;
		}
		// Check if any model has status "loaded"
		return response.find("\"loaded\"") != std::string::npos;
	}

	std::string getLoadedModelPath()
	{
		if (!isServerHealthy())
			return "";
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/models";
		auto [success, response] = httpClient_.get(url);
		if (!success) {
			spdlog::debug("getLoadedModelPath failed: {}", response);
			return "";
		}
		spdlog::debug("/models response: {}", response);
		// Find a model with status "loaded" and extract its "id"
		auto loadedPos = response.find("\"loaded\"");
		if (loadedPos == std::string::npos)
			return "";
		// Search backwards from "loaded" to find the nearest "id" field
		auto idPos = response.rfind("\"id\"", loadedPos);
		if (idPos == std::string::npos)
			return "";
		auto colonPos = response.find(":", idPos);
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
		// Get the currently loaded model name to send in the unload request
		std::string loadedModel = getLoadedModelPath();
		if (loadedModel.empty()) {
			spdlog::warn("No model loaded, nothing to unload");
			return false;
		}
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/models/unload";
		nlohmann::json bodyJson;
		bodyJson["model"] = loadedModel;
		auto [success, response] = httpClient_.post(url, bodyJson.dump());
		if (success) {
			spdlog::info("Model unloaded successfully");
		} else {
			spdlog::error("Failed to unload model: {}", response);
		}
		return success;
	}

	bool loadModel(const std::string &modelIdentifier)
	{
		if (!isServerHealthy())
			return false;
		// modelIdentifier is now the section name from models.ini (e.g.,
		// "orchestrator") Use it directly for the API call
		auto &cfg = ConfigManager::instance().getConfig();
		std::string url = "http://" + cfg.server.host + ":" +
						  std::to_string(cfg.server.port) + "/models/load";
		nlohmann::json bodyJson;
		bodyJson["model"] = modelIdentifier;
		auto [success, response] = httpClient_.post(url, bodyJson.dump());
		if (success) {
			spdlog::info("Model loaded: {}", modelIdentifier);
		} else {
			spdlog::error("Failed to load model: {}", response);
		}
		return success;
	}

  private:
	void readPipe(int fd)
	{
		const size_t bufferSize = 4096;
		char buffer[bufferSize];
		ssize_t bytesRead;
		std::string lineBuffer;

		while (!stopPipeReader_.load()) {
			// Use select() for timeout
			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(fd, &readfds);

			struct timeval tv = { 0, 100000 }; // 100ms timeout
			int sel = select(fd + 1, &readfds, NULL, NULL, &tv);
			if (sel <= 0) {
				continue;
			}

			bytesRead = read(fd, buffer, bufferSize - 1);
			if (bytesRead <= 0) {
				break;
			}

			buffer[bytesRead] = '\0';

			// Process character by character to handle line breaks
			for (ssize_t i = 0; i < bytesRead; ++i) {
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

		close(fd);
	}

	pid_t pid_;
	bool running_;
	std::function<void(const std::string &)> outputCallback_;
	std::thread pipeReadThread_;
	std::atomic<bool> stopPipeReader_{ false };
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