#include "gpu_monitor.h"
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

void GpuMonitor::update() {
	FILE *pipe = POPEN("nvidia-smi --query-gpu=memory.total,memory.used,memory.free --format=csv,noheader,nounits", "r");
	if (!pipe)
		return;

	std::vector<MemoryStats> new_stats;
	char buffer[256];
	while (std::fgets(buffer, sizeof(buffer), pipe)) {
		uint64_t total, used, free;
		if (std::sscanf(buffer, "%" SCNu64 ", %" SCNu64 ", %" SCNu64, &total, &used, &free) == 3) {
			MemoryStats s;
			s.total_mb = total;
			s.used_mb = used;
			s.available_mb = free;
			s.usage_percentage = (total > 0) ? (used * 100.0 / total) : 0.0;
			new_stats.push_back(s);
		}
	}
	PCLOSE(pipe);

	std::lock_guard<std::mutex> lock(stats_mutex_);
	stats_ = std::move(new_stats);
}

std::vector<MemoryStats> GpuMonitor::get_stats() const {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}
