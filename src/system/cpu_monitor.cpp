/**
 * @file cpu_monitor.cpp
 * @brief CPU monitor base implementation.
 *
 * Provides the platform-dispatched update() method and implements
 * common accessor methods for CPU statistics. Platform-specific
 * implementations are in cpu_linux.cpp and cpu_windows.cpp.
 */

#include "cpu_monitor.h"

ProcessorStats CpuMonitor::get_stats() const
{
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}

std::optional<ProcessorStats> CpuMonitor::try_update()
{
	update();
	return get_stats();
}
