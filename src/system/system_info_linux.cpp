#include "system_info.h"
#include "system_info_utils.h"
#include <fstream>
#include <string>

Hardware SystemInfo::get_cpu_info() {
	std::ifstream file("/proc/cpuinfo");

	std::string model_name;

	auto trim = [](const std::string &s) {
		auto start = s.find_first_not_of(" \t");
		auto end = s.find_last_not_of(" \t");
		return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
	};

	for (std::string line; std::getline(file, line);) {
		auto colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = trim(line.substr(0, colon));
		std::string value = trim(line.substr(colon + 1));

		if (key == "model name") {
			model_name = value;
			break;
		}
	}

	std::string make = "Unknown";
	std::string model = "Unknown";
	if (!model_name.empty()) {
		auto space = model_name.find(' ');
		make = model_name.substr(0, space);
		model = (space != std::string::npos) ? model_name.substr(space + 1) : "";
	}

	return {HardwareType::CPU, make, model};
}

void SystemInfo::update_linux() {
	cpu_ = get_cpu_info();
	gpus_ = SystemInfoUtils::get_gpu_info();
}

void SystemInfo::update() {
	update_linux();
}
