/**
 * @file gpuMonitor.cpp
 * @brief NVIDIA GPU monitoring implementation.
 *
 * Parses nvidia-smi CSV output to extract VRAM usage and GPU utilization
 * statistics for all detected NVIDIA GPUs. Provides thread-safe accessors
 * for both memory and load statistics.
 */

#include "gpuMonitor.h"

#include <spdlog/spdlog.h>
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
	if (!pipe) {
		spdlog::error("Failed to query GPU (nvidia-smi not found or error)");
		return;
	}

	std::vector<MemoryStats> newStats;
	std::vector<ProcessorStats> newLoadStats;

	char buffer[256];
	// Parse CSV output line by line
	while (std::fgets(buffer, sizeof(buffer), pipe)) {
		int index, utilPct;
		uint64_t total, used, free;
		// Parse: index, total_mb, used_mb, free_mb, utilization_pct
		if (std::sscanf(buffer,
						"%d, %" SCNu64 ", %" SCNu64 ", %" SCNu64 ", %d",
						&index,
						&total,
						&used,
						&free,
						&utilPct) == 5) {
			MemoryStats s;
			s.id = index;
			s.totalMb = total;
			s.usedMb = used;
			s.availableMb = free;
			// Calculate memory usage percentage
			s.usagePercentage = (total > 0) ? (used * 100.0 / total) : 0.0;
			newStats.push_back(s);

			ProcessorStats p;
			// GPU utilization is already a percentage from nvidia-smi
			p.usagePercentage = static_cast<double>(utilPct);
			newLoadStats.push_back(p);
		}
	}
	PCLOSE(pipe);

	// Update internal state under lock
	std::lock_guard<std::mutex> lock(statsMutex_);
	stats_ = std::move(newStats);
	loadStats_ = std::move(newLoadStats);
}

std::vector<MemoryStats> GpuMonitor::getStats() const
{
	std::lock_guard<std::mutex> lock(statsMutex_);
	return stats_;
}

std::vector<ProcessorStats> GpuMonitor::getLoadStats() const
{
	std::lock_guard<std::mutex> lock(statsMutex_);
	return loadStats_;
}
