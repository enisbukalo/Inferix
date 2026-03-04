#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Discriminates between CPU and GPU hardware entries.
 */
enum class HardwareType {
	CPU, ///< Central processing unit.
	GPU	 ///< Graphics processing unit.
};

/**
 * @brief Identifies a hardware device by type, manufacturer, and model name.
 */
struct Hardware {
	HardwareType type; ///< Whether this entry describes a CPU or GPU.
	std::string make;  ///< Manufacturer name (e.g. "Intel", "NVIDIA").
	std::string model; ///< Model / product name (e.g. "Core i9-13900K").
};

/**
 * @brief Singleton that detects and stores CPU and GPU hardware identifiers.
 *
 * Hardware detection is platform-dispatched at runtime. Call @ref update() once
 * during startup; subsequent reads via @ref get_cpu() and @ref get_gpus() are
 * cheap accessor calls.
 */
class SystemInfo {
  public:
	/**
	 * @brief Returns the process-wide singleton instance.
	 *
	 * @return Reference to the single @c SystemInfo object.
	 */
	static SystemInfo &instance() {
		static SystemInfo info;
		return info;
	}

	/**
	 * @brief Detects hardware on the current platform and populates internal state.
	 *
	 * Dispatches to the appropriate platform implementation
	 * (Linux, Windows, or a no-op fallback).
	 */
	void update();

	/**
	 * @brief Returns the detected CPU descriptor.
	 *
	 * @return A @c Hardware value whose @c type is @c HardwareType::CPU.
	 */
	Hardware get_cpu() const {
		return cpu_;
	}

	/**
	 * @brief Returns all detected GPU descriptors.
	 *
	 * @return A vector of @c Hardware values whose @c type is @c HardwareType::GPU.
	 *         Empty if no GPUs were detected.
	 */
	std::vector<Hardware> get_gpus() const {
		return gpus_;
	}

	/**
	 * @brief Attempts a hardware detection update.
	 *
	 * Calls @ref update() and returns the detected CPU descriptor on success.
	 *
	 * @return The CPU @c Hardware descriptor, or @c std::nullopt if detection failed.
	 */
	std::optional<Hardware> try_update();

  private:
	SystemInfo() = default;

	Hardware cpu_;
	std::vector<Hardware> gpus_;

	void update_linux();
	void update_windows();
	void update_unknown();

	Hardware get_cpu_info();
};
