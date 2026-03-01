#include "cpu_monitor.h"
#include <thread>
#include <windows.h>

void CpuMonitor::update_windows() {
	FILETIME prev_idle_time, prev_kernel_time, prev_user_time;
	FILETIME curr_idle_time, curr_kernel_time, curr_user_time;

	GetSystemTimes(&prev_idle_time, &prev_kernel_time, &prev_user_time);

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	GetSystemTimes(&curr_idle_time, &curr_kernel_time, &curr_user_time);

	ULARGE_INTEGER prev_idle, prev_total;
	ULARGE_INTEGER curr_idle, curr_total;

	prev_idle.LowPart = prev_idle_time.dwLowDateTime;
	prev_idle.HighPart = prev_idle_time.dwHighDateTime;

	prev_total.LowPart = prev_kernel_time.dwLowDateTime;
	prev_total.HighPart = prev_kernel_time.dwHighDateTime;

	curr_idle.LowPart = curr_idle_time.dwLowDateTime;
	curr_idle.HighPart = curr_idle_time.dwHighDateTime;

	curr_total.LowPart = curr_kernel_time.dwLowDateTime;
	curr_total.HighPart = curr_kernel_time.dwHighDateTime;

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	__int64 prev_idle_ms = (prev_idle.QuadPart * 1000) / frequency.QuadPart;
	__int64 prev_total_ms = (prev_total.QuadPart * 1000) / frequency.QuadPart;
	__int64 curr_idle_ms = (curr_idle.QuadPart * 1000) / frequency.QuadPart;
	__int64 curr_total_ms = (curr_total.QuadPart * 1000) / frequency.QuadPart;

	__int64 idle_delta = curr_idle_ms - prev_idle_ms;
	__int64 total_delta = curr_total_ms - prev_total_ms;

	if (total_delta > 0) {
		ProcessorStats new_stats;
		new_stats.usage_percentage = (1.0 - (double)idle_delta / total_delta) * 100.0;

		std::lock_guard<std::mutex> lock(stats_mutex_);
		stats_ = new_stats;
	}
}
