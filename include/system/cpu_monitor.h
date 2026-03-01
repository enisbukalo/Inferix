#pragma once
#include "processor_stats.h"
#include <mutex>
#include <optional>

class CpuMonitor {
  public:
	static CpuMonitor &instance() {
		static CpuMonitor monitor;
		return monitor;
	}

	void update();
	ProcessorStats get_stats() const;
	std::optional<ProcessorStats> try_update();

  private:
	CpuMonitor() = default;
	ProcessorStats stats_;
	mutable std::mutex stats_mutex_;

	void update_linux();
	void update_windows();
	void update_unknown();
};
