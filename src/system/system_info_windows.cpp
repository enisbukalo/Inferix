#include "system_info.h"
#include "system_info_utils.h"
#include <windows.h>
#include <string>

Hardware SystemInfo::get_cpu_info() {
	std::string model_name;

	HKEY key;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0, KEY_READ, &key) == ERROR_SUCCESS) {

		char buffer[256];
		DWORD size = sizeof(buffer);
		if (RegQueryValueExA(key, "ProcessorNameString", nullptr, nullptr,
			reinterpret_cast<LPBYTE>(buffer), &size) == ERROR_SUCCESS) {
			model_name = buffer;
		}
		RegCloseKey(key);
	}

	std::string make = "Unknown";
	std::string model = "Unknown";
	if (!model_name.empty()) {
		auto space = model_name.find(' ');
		make = model_name.substr(0, space);
		model = (space != std::string::npos) ? model_name.substr(space + 1) : "";
	}

	return { HardwareType::CPU, make, model };
}

void SystemInfo::update_windows() {
	cpu_ = get_cpu_info();
	gpus_ = SystemInfoUtils::get_gpu_info();
}

void SystemInfo::update() {
	update_windows();
}
