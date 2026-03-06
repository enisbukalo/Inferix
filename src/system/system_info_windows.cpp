/**
 * @file system_info_windows.cpp
 * @brief Windows-specific system information implementation.
 *
 * Implements CPU and GPU detection for Windows using the Windows Registry
 * and nvidia-smi command-line tool.
 */

#include "system_info.h"
#include "system_info_utils.h"
#include <string>
#include <windows.h>

Hardware SystemInfo::get_cpu_info()
{
	std::string model_name;

	// Open Windows Registry key for CPU information
	HKEY key;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
					  "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
					  0,
					  KEY_READ,
					  &key) == ERROR_SUCCESS) {

		char buffer[256];
		DWORD size = sizeof(buffer);
		// Read ProcessorNameString value from registry
		if (RegQueryValueExA(key,
							 "ProcessorNameString",
							 nullptr,
							 nullptr,
							 reinterpret_cast<LPBYTE>(buffer),
							 &size) == ERROR_SUCCESS) {
			model_name = buffer;
		}
		RegCloseKey(key);
	}

	// Parse "Intel Core i9-13900K" into make="Intel" and model="Core i9-13900K"
	std::string make = "Unknown";
	std::string model = "Unknown";
	if (!model_name.empty()) {
		auto space = model_name.find(' ');
		make = model_name.substr(0, space);
		model = (space != std::string::npos) ? model_name.substr(space + 1) : "";
	}

	return { HardwareType::CPU, make, model };
}

void SystemInfo::update_windows()
{
	cpu_ = get_cpu_info();
	gpus_ = SystemInfoUtils::get_gpu_info();
}

void SystemInfo::update()
{
	update_windows();
}
