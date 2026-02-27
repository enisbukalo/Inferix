#pragma once
#include <cstdint>
#include <string>
#include <optional>

struct RAMStats {
    uint64_t total_mb = 0;
    uint64_t used_mb = 0;
    uint64_t available_mb = 0;
    double usage_percentage = 0.0;
};

class MemoryMonitor {
public:
    static MemoryMonitor& instance() {
        static MemoryMonitor monitor;
        return monitor;
    }

    void update();
    RAMStats get_stats() const { return stats_; }
    std::optional<RAMStats> try_update();

private:
    MemoryMonitor() = default;

    RAMStats stats_;

    void update_linux();
    void update_windows();
    void update_unknown();
};
