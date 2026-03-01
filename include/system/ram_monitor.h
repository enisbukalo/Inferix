#pragma once
#include "ram_stats.h"
#include <optional>
#include <mutex>

class MemoryMonitor {
public:
    static MemoryMonitor& instance() {
        static MemoryMonitor monitor;
        return monitor;
    }

    void update();
    RAMStats get_stats() const;
    std::optional<RAMStats> try_update();

private:
    MemoryMonitor() = default;

    RAMStats stats_;
    mutable std::mutex stats_mutex_;

    void update_linux();
    void update_windows();
    void update_unknown();
};
