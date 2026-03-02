#pragma once
#include "processor_stats.h"
#include <mutex>
#include <optional>

/**
 * @brief Thread-safe singleton that measures CPU utilisation.
 *
 * CPU sampling is platform-dispatched. The latest snapshot is stored
 * internally and can be retrieved at any time via @ref get_stats().
 * All public accessors are safe to call from multiple threads concurrently.
 */
class CpuMonitor {
  public:
	/**
	 * @brief Returns the process-wide singleton instance.
	 *
	 * @return Reference to the single @c CpuMonitor object.
	 */
	static CpuMonitor &instance() {
		static CpuMonitor monitor;
		return monitor;
	}

	/**
	 * @brief Samples CPU usage and updates the internal snapshot.
	 *
	 * Dispatches to the appropriate platform implementation
	 * (Linux, Windows, or a no-op fallback).
	 */
	void update();

	/**
	 * @brief Returns the latest @c ProcessorStats snapshot under lock.
	 *
	 * @return A copy of the most recently sampled @c ProcessorStats.
	 */
	ProcessorStats get_stats() const;

	/**
	 * @brief Calls @ref update() and returns the resulting stats.
	 *
	 * @return The new @c ProcessorStats on success, or @c std::nullopt if sampling failed.
	 */
	std::optional<ProcessorStats> try_update();

  private:
	CpuMonitor() = default;
	ProcessorStats stats_;
	mutable std::mutex stats_mutex_;

	void update_linux();
	void update_windows();
	void update_unknown();
};
