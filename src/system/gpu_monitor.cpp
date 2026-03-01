#include "gpu_monitor.h"
#include <cstdio>
#include <cstdlib>

void GpuMonitor::update() {
    FILE* pipe = _popen("nvidia-smi --query-gpu=memory.total,memory.used,memory.free --format=csv,noheader,nounits", "r");
    if (!pipe) return;

    std::vector<MemoryStats> new_stats;
    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe)) {
        uint64_t total, used, free;
        if (std::sscanf(buffer, "%llu, %llu, %llu", &total, &used, &free) == 3) {
            MemoryStats s;
            s.total_mb       = total;
            s.used_mb        = used;
            s.available_mb   = free;
            s.usage_percentage = (total > 0) ? (used * 100.0 / total) : 0.0;
            new_stats.push_back(s);
        }
    }
    _pclose(pipe);

    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = std::move(new_stats);
}

std::vector<MemoryStats> GpuMonitor::get_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}
