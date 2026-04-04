/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Minimal main function that delegates all functionality to App::Run().
 * This file serves as the compilation entry point for the Workbench TUI.
 */

#include "app.h"
#include "configManager.h"
#include "llamaServerProcess.h"
#include "systemMonitorRunner.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>
#include <sstream>

/**
 * @brief Cleans up old log files based on retention policy.
 * @param logDir Path to the logs directory.
 * @param retentionDays Number of days to retain log files.
 */
void cleanupOldLogs(const std::string &logDir, int retentionDays)
{
	if (retentionDays <= 0) {
		spdlog::debug("Log cleanup disabled (retention: 0)");
		return;
	}

	spdlog::info("Starting log cleanup (retention: {} days)", retentionDays);

	try {
		std::filesystem::path logsPath(logDir);
		if (!std::filesystem::exists(logsPath)) {
			spdlog::debug("No log files to clean up");
			return;
		}

		auto now = std::chrono::system_clock::now();
		int deletedCount = 0;

		for (const auto &entry : std::filesystem::directory_iterator(logsPath)) {
			if (!entry.is_regular_file())
				continue;

			std::string filename = entry.path().filename().string();
			if (filename.find("_workbench.log") == std::string::npos)
				continue;

			auto fileTime = std::filesystem::last_write_time(entry.path());
			auto fileTimeSys = std::chrono::system_clock::from_time_t(
				std::chrono::duration_cast<std::chrono::seconds>(
					fileTime.time_since_epoch())
					.count());

			auto age = std::chrono::duration_cast<std::chrono::hours>(
				now - fileTimeSys);
			if (age.count() > retentionDays * 24) {
				std::filesystem::remove(entry.path());
				deletedCount++;
			}
		}

		if (deletedCount > 0)
			spdlog::info("Cleaned up {} old log file(s)", deletedCount);
		else
			spdlog::debug("No log files to clean up");
	} catch (const std::exception &e) {
		spdlog::warn("Failed to clean up old logs: {}", e.what());
	}
}

/**
 * @brief Program entry point.
 *
 * Loads user configuration and launches the Workbench terminal UI.
 * The SystemMonitorRunner is started from within App::run() after the
 * screen is created, so it can trigger redraws when monitor data updates.
 *
 * @return 0 on successful execution.
 */
int main()
{
	try {
		// Create logs directory if it doesn't exist
		std::filesystem::path logsDir = ConfigManager::instance().getLogsDir();
		std::filesystem::create_directories(logsDir);

		// Generate timestamp for log filename
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
		ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
		std::string logFilename = ss.str() + "_workbench.log";

		// Create rotating file sink - 5MB max, keep 3 files
		auto rotatingSink =
			std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
				(logsDir / logFilename).string(),
				1024 * 1024 * 5,
				3);

		// Create logger with the file sink
		std::vector<spdlog::sink_ptr> sinks{ rotatingSink };
		auto logger = std::make_shared<spdlog::logger>("workbench",
													   sinks.begin(),
													   sinks.end());

		// Set pattern: [timestamp] [level] message
		logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%-7l%$] %v");

		// Set default level to debug (capture everything)
		logger->set_level(spdlog::level::debug);

		// Register as default logger
		spdlog::set_default_logger(logger);

		// Flush immediately to ensure log is written
		logger->flush();
		spdlog::info("Workbench starting...");
	} catch (const spdlog::spdlog_ex &ex) {
		// Fallback to basic console if file sink fails
		spdlog::set_level(spdlog::level::debug);
		spdlog::warn("Failed to initialize file logging, using default: {}",
					 ex.what());
	}

	// Load configuration from disk
	ConfigManager::instance().load();

	// Clean up old log files based on retention policy
	cleanupOldLogs(ConfigManager::instance().getLogsDir(),
				   ConfigManager::instance().getConfig().ui.logRetentionDays);

	// Run the main application UI loop
	App::run();

	// Clean up the system monitor before exit
	SystemMonitorRunner::instance().stop();

	// Terminate llama-server if still running
	LlamaServerProcess::instance().terminate();

	// Flush logs before exit
	spdlog::default_logger()->flush();
	spdlog::info("Workbench exiting.");
	return 0;
}
