#pragma once
#include <cstdint>

/**
 * @brief Snapshot of processor utilisation for a single CPU or GPU compute
 * engine.
 */
struct ProcessorStats
{
	double usage_percentage =
		0.0; ///< Processor load as a percentage of total capacity (0–100 %).
};
