#pragma once
#include "memory_stats.h"
#include <mutex>
#include <optional>

/**
 * @brief Thread-safe singleton that measures system RAM utilisation.
 *
 * RAM sampling is platform-dispatched. The latest snapshot is stored
 * internally and can be retrieved at any time via @ref get_stats().
 * All public accessors are safe to call from multiple threads concurrently.
 */
class MemoryMonitor
{
  public:
	/**
	 * @brief Returns the process-wide singleton instance.
	 *
	 * @return Reference to the single @c MemoryMonitor object.
	 */
	static MemoryMonitor &instance()
	{
		static MemoryMonitor monitor;
		return monitor;
	}

	/**
	 * @brief Samples system RAM usage and updates the internal snapshot.
	 *
	 * Dispatches to the appropriate platform implementation
	 * (Linux, Windows, or a no-op fallback).
	 */
	void update();

	/**
	 * @brief Returns the latest @c MemoryStats snapshot under lock.
	 *
	 * @return A copy of the most recently sampled @c MemoryStats.
	 */
	MemoryStats get_stats() const;

	/**
	 * @brief Calls @ref update() and returns the resulting stats.
	 *
	 * @note Not yet fully implemented; may return @c std::nullopt as a
	 * placeholder.
	 *
	 * @return The new @c MemoryStats on success, or @c std::nullopt if sampling
	 * failed.
	 */
	std::optional<MemoryStats> try_update();

  private:
	MemoryMonitor() = default;

	MemoryStats stats_;
	mutable std::mutex stats_mutex_;

	void update_linux();
	void update_windows();
	void update_unknown();
};
