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
	FILE *pipe = POPEN("nvidia-smi --query-gpu=index,memory.total,memory.used,memory.free,utilization.gpu"
					   " --format=csv,noheader,nounits",
					   "r");
	if (!pipe)
		return;

	std::vector<MemoryStats> new_stats;
	std::vector<ProcessorStats> new_load_stats;

	char buffer[256];
	while (std::fgets(buffer, sizeof(buffer), pipe)) {
		int index, util_pct;
		uint64_t total, used, free;
		if (std::sscanf(buffer, "%d, %" SCNu64 ", %" SCNu64 ", %" SCNu64 ", %d", &index, &total, &used, &free, &util_pct) == 5) {
			MemoryStats s;
			s.id = index;
			s.total_mb = total;
			s.used_mb = used;
			s.available_mb = free;
			s.usage_percentage = (total > 0) ? (used * 100.0 / total) : 0.0;
			new_stats.push_back(s);

			ProcessorStats p;
			p.usage_percentage = static_cast<double>(util_pct);
			new_load_stats.push_back(p);
		}
	}
	PCLOSE(pipe);

	std::lock_guard<std::mutex> lock(stats_mutex_);
	stats_ = std::move(new_stats);
	load_stats_ = std::move(new_load_stats);
}

std::vector<MemoryStats> GpuMonitor::get_stats() const {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}

std::vector<ProcessorStats> GpuMonitor::get_load_stats() const {
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return load_stats_;
}
