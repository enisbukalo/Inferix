#pragma once
#include <cstdint>

/**
 * @file processor_stats.h
 * @brief Snapshot of processor utilisation for a single CPU or GPU compute engine.
 *
 * This structure captures a point-in-time snapshot of processor load
 * statistics. The usage percentage represents the fraction of time the
 * processor spent executing non-idle tasks during the sampling interval.
 *
 * @note For CPU monitoring, this is calculated by comparing idle time
 *       deltas over a sampling period. For GPU monitoring, this value
 *       is obtained directly from nvidia-smi.
 */
struct ProcessorStats
{
	double usage_percentage = 0.0; ///< Processor load as a percentage of total capacity (0–100 %).
};
