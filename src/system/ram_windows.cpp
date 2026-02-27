#include "ram_monitor.h"
#include <windows.h>

void MemoryMonitor::update_windows() {
    MEMORYSTATUSEX statex{};
    statex.dwLength = sizeof(statex);

    if (GlobalMemoryStatusEx(&statex)) {
        stats_.total_bytes = statex.ullTotalPhys;
        stats_.used_bytes = statex.ullTotalPhys - statex.ullAvailPhys;
        stats_.available_bytes = statex.ullAvailPhys;
        stats_.usage_percentage = stats_.total_bytes > 0
            ? (stats_.used_bytes * 100.0 / stats_.total_bytes)
            : 0.0;
    }
}

void MemoryMonitor::update() {
    update_windows();
}