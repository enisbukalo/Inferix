#include "ram_monitor.h"

MemoryStats MemoryMonitor::get_stats() const {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}
