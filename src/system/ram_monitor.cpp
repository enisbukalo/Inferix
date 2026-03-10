/**
 * @file ramMonitor.cpp
 * @brief RAM monitor base implementation.
 *
 * Provides the platform-dispatched update() method and implements
 * common accessor methods for memory statistics. Platform-specific
 * implementations are in ramLinux.cpp and ramWindows.cpp.
 */

#include "ram_monitor.h"

MemoryStats MemoryMonitor::getStats() const
{
	std::lock_guard<std::mutex> lock(statsMutex_);
	return stats_;
}
