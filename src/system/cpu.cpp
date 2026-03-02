#include "cpu_monitor.h"

void CpuMonitor::update() {
#ifdef _WIN32
	update_windows();
#elif __linux__
	update_linux();
#else
	update_unknown();
#endif
}
