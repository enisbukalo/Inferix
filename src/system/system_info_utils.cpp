#include "system_info_utils.h"
#include <cstdio>
#include <string>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#define NULL_REDIRECT "2>nul"
#else
#define POPEN popen
#define PCLOSE pclose
#define NULL_REDIRECT "2>/dev/null"
#endif

std::vector<Hardware> SystemInfoUtils::get_gpu_info() {
	std::vector<Hardware> gpus;

	FILE *pipe = POPEN("nvidia-smi --query-gpu=name --format=csv,noheader " NULL_REDIRECT, "r");
	if (!pipe)
		return gpus;

	char buffer[256];
	while (fgets(buffer, sizeof(buffer), pipe)) {
		std::string name(buffer);
		if (!name.empty() && name.back() == '\n')
			name.pop_back();
		if (!name.empty() && name.back() == '\r')
			name.pop_back();
		if (name.empty())
			continue;

		auto space = name.find(' ');
		std::string make = name.substr(0, space);
		std::string model = (space != std::string::npos) ? name.substr(space + 1) : "";
		gpus.push_back({HardwareType::GPU, make, model});
	}

	PCLOSE(pipe);
	return gpus;
}
