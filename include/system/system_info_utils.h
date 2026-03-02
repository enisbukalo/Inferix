#pragma once
#include "system_info.h"
#include <vector>

/**
 * @brief Utility functions for querying hardware information from the system.
 */
namespace SystemInfoUtils {

/**
 * @brief Runs @c nvidia-smi to enumerate GPU make and model strings.
 *
 * Parses the CSV output of @c nvidia-smi to build a @c Hardware descriptor
 * for each detected NVIDIA GPU. If @c nvidia-smi is unavailable or returns
 * a non-zero exit code, an empty vector is returned gracefully.
 *
 * @return A vector of @c Hardware entries with @c type set to
 *         @c HardwareType::GPU, one entry per detected GPU.
 *         Empty if @c nvidia-smi is unavailable.
 */
std::vector<Hardware> get_gpu_info();

} // namespace SystemInfoUtils
