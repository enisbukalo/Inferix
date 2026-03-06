/**
 * @file cpu_linux.cpp
 * @brief Linux-specific CPU monitoring implementation.
 *
 * Parses /proc/stat to extract CPU time statistics and calculates
 * utilization by comparing idle time deltas over a 100ms sampling interval.
 */

#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <thread>

void CpuMonitor::update_linux() {
	std::ifstream file("/proc/stat");

	long long prev_idle = 0, prev_total = 0;

	std::string line;
	// Read first snapshot from /proc/stat
	if (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string cpu;
		iss >> cpu;

		// Parse CPU time fields: user, nice, system, idle, iowait, irq, softirq, steal
		long long user, nice, system, idle, iowait, irq, softirq, steal;
		iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

		// Idle time includes both idle and iowait states
		prev_idle = idle + iowait;
		// Total time is sum of all CPU time fields
		prev_total = user + nice + system + idle + iowait + irq + softirq + steal;
	}

	// Wait for a short interval to measure CPU activity
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Reset file stream and re-read
	file.clear();
	file.seekg(0);

	long long curr_idle = 0, curr_total = 0;

	// Read second snapshot
	if (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string cpu;
		iss >> cpu;

		long long user, nice, system, idle, iowait, irq, softirq, steal;
		iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

		curr_idle = idle + iowait;
		curr_total = user + nice + system + idle + iowait + irq + softirq + steal;
	}

	// Calculate the change in idle and total time during the interval
	long long idle_delta = curr_idle - prev_idle;
	long long total_delta = curr_total - prev_total;

	if (total_delta > 0) {
		ProcessorStats new_stats;
		// CPU usage = 1 - (idle_time / total_time), expressed as percentage
		new_stats.usage_percentage = ((double)idle_delta / total_delta);

		std::lock_guard<std::mutex> lock(stats_mutex_);
		stats_ = new_stats;
	}
}
