#include "ram_monitor.h"
#include <fstream>
#include <sstream>

void MemoryMonitor::update_linux() {
    std::ifstream file("/proc/meminfo");

    uint64_t total = 0, available = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value, unit;
        iss >> key >> value >> unit;

        if (key == "MemTotal:") {
            total = std::stoull(value) * 1024;
        } else if (key == "MemAvailable:") {
            available = std::stoull(value) * 1024;
        }
    }

    stats_.total_bytes = total;
    stats_.available_bytes = available;
    stats_.used_bytes = total - available;
    stats_.usage_percentage = total > 0 ? (stats_.used_bytes * 100.0 / total) : 0.0;
}

void MemoryMonitor::update() {
    update_linux();
}