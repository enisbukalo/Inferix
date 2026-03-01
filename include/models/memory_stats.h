#pragma once
#include <cstdint>

struct MemoryStats {
    uint64_t total_mb        = 0;
    uint64_t used_mb         = 0;
    uint64_t available_mb    = 0;
    double   usage_percentage = 0.0;
};
