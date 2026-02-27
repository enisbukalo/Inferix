#include "ram_monitor.h"
#include <fstream>
#include <sstream>
#include <cmath>

void MemoryMonitor::update_linux() {
    std::ifstream file("/proc/meminfo");

    uint64_t total = 0, available = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value, unit;
        iss >> key >> value >> unit;

        if (key == "MemTotal:") {
            total = std::round(std::stoull(value) / 1024.0);
        } else if (key == "MemAvailable:") {
            available = std::round(std::stoull(value) / 1024.0);
        }
    }

    stats_.total_mb = total;
    stats_.available_mb = available;
    stats_.used_mb = total - available;
    stats_.usage_percentage = total > 0 ? (stats_.used_mb * 100.0 / total) : 0.0;
}

void MemoryMonitor::update() {
    update_linux();
}