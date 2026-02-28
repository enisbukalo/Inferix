#pragma once
#include "system_info.h"
#include <vector>

namespace SystemInfoUtils {
    std::vector<Hardware> get_gpu_info();
}
