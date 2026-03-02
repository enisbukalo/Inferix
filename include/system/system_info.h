#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

enum class HardwareType { CPU,
						  GPU };

struct Hardware {
	HardwareType type;
	std::string make;
	std::string model;
};

class SystemInfo {
  public:
	static SystemInfo &instance() {
		static SystemInfo info;
		return info;
	}

	void update();
	Hardware get_cpu() const { return cpu_; }
	std::vector<Hardware> get_gpus() const { return gpus_; }
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
