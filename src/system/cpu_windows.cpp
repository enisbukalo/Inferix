/**
 * @file cpu_windows.cpp
 * @brief Windows-specific CPU monitoring implementation.
 *
 * Uses Windows API GetSystemTimes() to measure CPU utilization by
 * comparing idle time deltas over a 50ms sampling interval.
 */

#include "cpu_monitor.h"
#include <thread>
#include <windows.h>

void CpuMonitor::update_windows() {
	// Take first snapshot of system times
	FILETIME prev_idle_time, prev_kernel_time, prev_user_time;
	FILETIME curr_idle_time, curr_kernel_time, curr_user_time;

	GetSystemTimes(&prev_idle_time, &prev_kernel_time, &prev_user_time);

	// Wait for a short interval to measure CPU activity
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Take second snapshot after the interval
	GetSystemTimes(&curr_idle_time, &curr_kernel_time, &curr_user_time);

	// Convert FILETIME structures to ULARGE_INTEGER for easier arithmetic
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

	// Get performance counter frequency for conversion to milliseconds
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	// Convert from 100-nanosecond units to milliseconds
	__int64 prev_idle_ms = (prev_idle.QuadPart * 1000) / frequency.QuadPart;
	__int64 prev_total_ms = (prev_total.QuadPart * 1000) / frequency.QuadPart;
	__int64 curr_idle_ms = (curr_idle.QuadPart * 1000) / frequency.QuadPart;
	__int64 curr_total_ms = (curr_total.QuadPart * 1000) / frequency.QuadPart;

	// Calculate the change in idle and total time during the interval
	__int64 idle_delta = curr_idle_ms - prev_idle_ms;
	__int64 total_delta = curr_total_ms - prev_total_ms;

	if (total_delta > 0) {
		ProcessorStats new_stats;
		// CPU usage = 1 - (idle_time / total_time), expressed as percentage
		new_stats.usage_percentage = (1.0 - (double)idle_delta / total_delta) * 100.0;

		std::lock_guard<std::mutex> lock(stats_mutex_);
		stats_ = new_stats;
	}
}
