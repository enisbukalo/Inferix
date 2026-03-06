/**
 * @file ram_monitor.cpp
 * @brief RAM monitor base implementation.
 *
 * Provides the platform-dispatched update() method and implements
 * common accessor methods for memory statistics. Platform-specific
 * implementations are in ram_linux.cpp and ram_windows.cpp.
 */

#include "ram_monitor.h"

MemoryStats MemoryMonitor::get_stats() const
{
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}
