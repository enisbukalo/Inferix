#pragma once
#include "memory_stats.h"
#include "processor_stats.h"
#include <mutex>
#include <vector>

/**
 * @brief Thread-safe singleton that queries NVIDIA GPU statistics via @c nvidia-smi.
 *
 * Both VRAM usage (@ref get_stats()) and compute utilisation (@ref get_load_stats())
 * are updated together during each call to @ref update(). All public accessors are
 * safe to call from multiple threads concurrently.
 */
class GpuMonitor {
  public:
	/**
	 * @brief Returns the process-wide singleton instance.
	 *
	 * @return Reference to the single @c GpuMonitor object.
	 */
	static GpuMonitor &instance() {
		static GpuMonitor monitor;
		return monitor;
	}

	/**
	 * @brief Queries all GPUs and updates the internal VRAM and load snapshots.
	 *
	 * Parses @c nvidia-smi CSV output for every detected GPU. If @c nvidia-smi is
	 * unavailable the internal stats remain unchanged.
	 */
	void update();

	/**
	 * @brief Returns per-GPU VRAM usage snapshots.
	 *
	 * @return A vector of @c MemoryStats, one entry per detected GPU, ordered by
	 *         device index. Empty if no GPUs were detected.
	 */
	std::vector<MemoryStats> get_stats() const;

	/**
	 * @brief Returns per-GPU compute utilisation snapshots.
	 *
	 * @return A vector of @c ProcessorStats, one entry per detected GPU, ordered by
	 *         device index. Empty if no GPUs were detected.
	 */
	std::vector<ProcessorStats> get_load_stats() const;

  private:
	GpuMonitor() = default;
	std::vector<MemoryStats> stats_;
	std::vector<ProcessorStats> load_stats_;
	mutable std::mutex stats_mutex_;
};
