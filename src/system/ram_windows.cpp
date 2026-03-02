#include "ram_monitor.h"
#include <cmath>
#include <windows.h>

void MemoryMonitor::update_windows() {
	MEMORYSTATUSEX statex{};
	statex.dwLength = sizeof(statex);

	if (GlobalMemoryStatusEx(&statex)) {
		MemoryStats new_stats;
		new_stats.total_mb = std::round(statex.ullTotalPhys / 1048576.0);
		new_stats.used_mb = std::round((statex.ullTotalPhys - statex.ullAvailPhys) / 1048576.0);
		new_stats.available_mb = std::round(statex.ullAvailPhys / 1048576.0);
		new_stats.usage_percentage = (new_stats.total_mb > 0)
										 ? (new_stats.used_mb * 100.0 / new_stats.total_mb)
										 : 0.0;

		std::lock_guard<std::mutex> lock(stats_mutex_);
		stats_ = new_stats;
	}
}

void MemoryMonitor::update() {
	update_windows();
}