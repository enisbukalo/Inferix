#include "cpu_monitor.h"

ProcessorStats CpuMonitor::get_stats() const {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}

std::optional<ProcessorStats> CpuMonitor::try_update() {
	update();
	return get_stats();
}
