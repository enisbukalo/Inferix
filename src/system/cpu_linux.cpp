#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <thread>

void CpuMonitor::update_linux() {
	std::ifstream file("/proc/stat");

	long long prev_idle = 0, prev_total = 0;

	std::string line;
	if (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string cpu;
		iss >> cpu;

		long long user, nice, system, idle, iowait, irq, softirq, steal;
		iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

		prev_idle = idle + iowait;
		prev_total = user + nice + system + idle + iowait + irq + softirq + steal;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	file.clear();
	file.seekg(0);

	long long curr_idle = 0, curr_total = 0;

	if (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string cpu;
		iss >> cpu;

		long long user, nice, system, idle, iowait, irq, softirq, steal;
		iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

		curr_idle = idle + iowait;
		curr_total = user + nice + system + idle + iowait + irq + softirq + steal;
	}

	long long idle_delta = curr_idle - prev_idle;
	long long total_delta = curr_total - prev_total;

	if (total_delta > 0) {
		ProcessorStats new_stats;
		new_stats.usage_percentage = ((double)idle_delta / total_delta);

		std::lock_guard<std::mutex> lock(stats_mutex_);
		stats_ = new_stats;
	}
}
