/**
 * @file cpuLinux.cpp
 * @brief Linux-specific CPU monitoring implementation.
 *
 * Parses /proc/stat to extract CPU time statistics and calculates
 * utilization by comparing idle time deltas over a 100ms sampling interval.
 */

#include "cpuMonitor.h"
#include <fstream>
#include <sstream>
#include <thread>

void CpuMonitor::updateLinux()
{
	std::ifstream file("/proc/stat");

	long long prevIdle = 0, prevTotal = 0;

	std::string line;
	// Read first snapshot from /proc/stat
	if (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string cpu;
		iss >> cpu;

		// Parse CPU time fields: user, nice, system, idle, iowait, irq, softirq,
		// steal
		long long user, nice, system, idle, iowait, irq, softirq, steal;
		iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >>
			steal;

		// Idle time includes both idle and iowait states
		prevIdle = idle + iowait;
		// Total time is sum of all CPU time fields
		prevTotal = user + nice + system + idle + iowait + irq + softirq + steal;
	}

	// Wait for a short interval to measure CPU activity
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Reset file stream and re-read
	file.clear();
	file.seekg(0);

	long long currIdle = 0, currTotal = 0;

	// Read second snapshot
	if (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string cpu;
		iss >> cpu;

		long long user, nice, system, idle, iowait, irq, softirq, steal;
		iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >>
			steal;

		currIdle = idle + iowait;
		currTotal = user + nice + system + idle + iowait + irq + softirq + steal;
	}

	// Calculate the change in idle and total time during the interval
	long long idleDelta = currIdle - prevIdle;
	long long totalDelta = currTotal - prevTotal;

	if (totalDelta > 0) {
		ProcessorStats newStats;
		// CPU usage = 1 - (idle_time / total_time), expressed as percentage
		newStats.usagePercentage = ((double)idleDelta / totalDelta);

		std::lock_guard<std::mutex> lock(statsMutex_);
		stats_ = newStats;
	}
}
