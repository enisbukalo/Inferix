/**
 * @file systemMonitorRunner.cpp
 * @brief Background monitoring thread implementation.
 *
 * Implements the polling thread that periodically updates all system
 * monitors (CPU, GPU, RAM) and triggers UI redraws via FTXUI events.
 */

#include "systemMonitorRunner.h"
#include "cpuMonitor.h"
#include "gpuMonitor.h"
#include "ramMonitor.h"
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

SystemMonitorRunner::SystemMonitorRunner(ftxui::ScreenInteractive &screen)
	: screen_(screen), thread_(&SystemMonitorRunner::run, this)
{
}

SystemMonitorRunner::~SystemMonitorRunner()
{
	stop();
}

void SystemMonitorRunner::stop()
{
	stopFlag_.store(true);
	cv_.notify_one();
	if (thread_.joinable())
		thread_.join();
}

void SystemMonitorRunner::run()
{
	while (true) {
		{
			std::unique_lock<std::mutex> lock(cvMutex_);
			cv_.wait_for(lock,
						 std::chrono::milliseconds(kThreadWaitTimeMs),
						 [this] { return stopFlag_.load(); });
		}
		if (stopFlag_.load())
			break;
		MemoryMonitor::instance().update();
		CpuMonitor::instance().update();
		GpuMonitor::instance().update();
		screen_.PostEvent(ftxui::Event::Custom);
	}
}
