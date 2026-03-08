/**
 * @file gpu_monitor.cpp
 * @brief NVIDIA GPU monitoring implementation.
 *
 * Parses nvidia-smi CSV output to extract VRAM usage and GPU utilization
 * statistics for all detected NVIDIA GPUs. Provides thread-safe accessors
 * for both memory and load statistics.
 */

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

void GpuMonitor::update()
{
	// Execute nvidia-smi with CSV output format for GPU statistics
	// Query: index, memory.total, memory.used, memory.free, utilization.gpu
	FILE *pipe = POPEN(
		"nvidia-smi "
		"--query-gpu=index,memory.total,memory.used,memory.free,utilization.gpu"
		" --format=csv,noheader,nounits",
		"r");
	if (!pipe)
		return;

	std::vector<MemoryStats> new_stats;
	std::vector<ProcessorStats> new_load_stats;

	char buffer[256];
	// Parse CSV output line by line
	while (std::fgets(buffer, sizeof(buffer), pipe)) {
		int index, util_pct;
		uint64_t total, used, free;
		// Parse: index, total_mb, used_mb, free_mb, utilization_pct
		if (std::sscanf(buffer,
						"%d, %" SCNu64 ", %" SCNu64 ", %" SCNu64 ", %d",
						&index,
						&total,
						&used,
						&free,
						&util_pct) == 5) {
			MemoryStats s;
			s.id = index;
			s.total_mb = total;
			s.used_mb = used;
			s.available_mb = free;
			// Calculate memory usage percentage
			s.usage_percentage = (total > 0) ? (used * 100.0 / total) : 0.0;
			new_stats.push_back(s);

			ProcessorStats p;
			// GPU utilization is already a percentage from nvidia-smi
			p.usage_percentage = static_cast<double>(util_pct);
			new_load_stats.push_back(p);
		}
	}
	PCLOSE(pipe);

	// Update internal state under lock
	std::lock_guard<std::mutex> lock(stats_mutex_);
	stats_ = std::move(new_stats);
	load_stats_ = std::move(new_load_stats);
}

std::vector<MemoryStats> GpuMonitor::get_stats() const
{
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return stats_;
}

std::vector<ProcessorStats> GpuMonitor::get_load_stats() const
{
	std::lock_guard<std::mutex> lock(stats_mutex_);
	return load_stats_;
}
