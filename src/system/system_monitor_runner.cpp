#include "system_monitor_runner.h"
#include "ram_monitor.h"
#include "gpu_monitor.h"
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

SystemMonitorRunner::SystemMonitorRunner(ftxui::ScreenInteractive& screen)
    : screen_(screen), thread_(&SystemMonitorRunner::run, this) {}

SystemMonitorRunner::~SystemMonitorRunner() { stop(); }

void SystemMonitorRunner::stop() {
    stop_flag_.store(true);
    cv_.notify_one();
    if (thread_.joinable()) thread_.join();
}

void SystemMonitorRunner::run() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(cv_mutex_);
            cv_.wait_for(lock, std::chrono::seconds(1),
                         [this] { return stop_flag_.load(); });
        }
        if (stop_flag_.load()) break;
        MemoryMonitor::instance().update();
        GpuMonitor::instance().update();
        screen_.PostEvent(ftxui::Event::Custom);
    }
}
