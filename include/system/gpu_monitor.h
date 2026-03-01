#pragma once
#include "memory_stats.h"
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

  private:
	GpuMonitor() = default;
	std::vector<MemoryStats> stats_;
	mutable std::mutex stats_mutex_;
};
