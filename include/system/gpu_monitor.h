#pragma once
#include "memory_stats.h"
#include "processor_stats.h"
#include <mutex>
#include <vector>

class GpuMonitor {
  public:
	static GpuMonitor &instance() {
		static GpuMonitor monitor;
		return monitor;
	}

	void update();
	std::vector<MemoryStats> get_stats() const;
	std::vector<ProcessorStats> get_load_stats() const;

  private:
	GpuMonitor() = default;
	std::vector<MemoryStats> stats_;
	std::vector<ProcessorStats> load_stats_;
	mutable std::mutex stats_mutex_;
};
