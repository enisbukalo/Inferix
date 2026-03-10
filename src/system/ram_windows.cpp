/**
 * @file ram_windows.cpp
 * @brief Windows-specific RAM monitoring implementation.
 *
 * Uses Windows API GlobalMemoryStatusEx() to query system memory
 * statistics and converts bytes to mebibytes for display.
 */

#include "ram_monitor.h"
#include <cmath>
#include <windows.h>

void MemoryMonitor::update_windows()
{
	MEMORYSTATUSEX statex{};
	statex.dwLength = sizeof(statex);

	// Query Windows API for memory statistics (values in bytes)
	if (GlobalMemoryStatusEx(&statex)) {
		MemoryStats new_stats;
		// Convert bytes to MiB (1 MiB = 1048576 bytes)
		new_stats.totalMb = std::round(statex.ullTotalPhys / 1048576.0);
		new_stats.usedMb =
			std::round((statex.ullTotalPhys - statex.ullAvailPhys) / 1048576.0);
		new_stats.availableMb = std::round(statex.ullAvailPhys / 1048576.0);
		// Calculate usage percentage
		new_stats.usagePercentage =
			(new_stats.totalMb > 0)
				? (new_stats.usedMb * 100.0 / new_stats.totalMb)
				: 0.0;

		std::lock_guard<std::mutex> lock(stats_mutex_);
		stats_ = new_stats;
	}
}

void MemoryMonitor::update()
{
	update_windows();
}