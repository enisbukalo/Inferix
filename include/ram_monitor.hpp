#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <iomanip>
#include <sstream>

struct RAMStats {
    uint64_t total_bytes = 0;
    uint64_t used_bytes = 0;
    uint64_t available_bytes = 0;
    double usage_percentage = 0.0;

    std::string to_string() const;
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