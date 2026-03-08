/**
 * @file ram_linux.cpp
 * @brief Linux-specific RAM monitoring implementation.
 *
 * Parses /proc/meminfo to extract memory statistics and converts
 * kilobytes to mebibytes for display.
 */

#include "ram_monitor.h"
#include <cmath>
#include <fstream>
#include <sstream>

void MemoryMonitor::update_linux()
{
	std::ifstream file("/proc/meminfo");

	uint64_t total = 0, available = 0;

	std::string line;
	// Parse /proc/meminfo line by line
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string key, value, unit;
		iss >> key >> value >> unit;

		// Extract MemTotal and MemAvailable fields (values in kB)
		if (key == "MemTotal:") {
			// Convert kB to MiB
			total = std::round(std::stoull(value) / 1024.0);
		} else if (key == "MemAvailable:") {
			// Convert kB to MiB
			available = std::round(std::stoull(value) / 1024.0);
		}
	}

	MemoryStats new_stats;
	new_stats.total_mb = total;
	new_stats.available_mb = available;
	new_stats.used_mb = total - available;
	// Calculate usage percentage
	new_stats.usage_percentage =
		total > 0 ? (new_stats.used_mb * 100.0 / total) : 0.0;

	std::lock_guard<std::mutex> lock(stats_mutex_);
	stats_ = new_stats;
}

void MemoryMonitor::update()
{
	update_linux();
}