#pragma once
#include <cstdint>

/**
 * @brief Snapshot of memory usage for a single hardware device (RAM or GPU
 * VRAM).
 *
 * All capacity values are expressed in mebibytes (MiB).
 */
struct MemoryStats
{
	int id = 0;				   ///< Zero-based device index.
	uint64_t total_mb = 0;	   ///< Total memory capacity in MiB.
	uint64_t used_mb = 0;	   ///< Currently used memory in MiB.
	uint64_t available_mb = 0; ///< Currently free memory in MiB.
	double usage_percentage =
		0.0; ///< Used memory as a percentage of total (0–100 %).
};
