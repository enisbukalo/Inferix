#include "ram_monitor.h"
#include <windows.h>
#include <cmath>

void MemoryMonitor::update_windows() {
    MEMORYSTATUSEX statex{};
    statex.dwLength = sizeof(statex);

    if (GlobalMemoryStatusEx(&statex)) {
        stats_.total_mb = std::round(statex.ullTotalPhys / 1048576.0);
        stats_.used_mb = std::round((statex.ullTotalPhys - statex.ullAvailPhys) / 1048576.0);
        stats_.available_mb = std::round(statex.ullAvailPhys / 1048576.0);
        stats_.usage_percentage = (stats_.total_mb > 0)
            ? (stats_.used_mb * 100.0 / stats_.total_mb)
            : 0.0;
    }
}

void MemoryMonitor::update() {
    update_windows();
}