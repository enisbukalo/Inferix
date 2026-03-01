#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ftxui { class ScreenInteractive; }

class SystemMonitorRunner {
public:
    explicit SystemMonitorRunner(ftxui::ScreenInteractive& screen);
    SystemMonitorRunner(const SystemMonitorRunner&) = delete;
    SystemMonitorRunner& operator=(const SystemMonitorRunner&) = delete;
    ~SystemMonitorRunner();
    void stop();

private:
    void run();

    ftxui::ScreenInteractive& screen_;
    std::atomic<bool>         stop_flag_{false};
    std::mutex                cv_mutex_;
    std::condition_variable   cv_;
    std::thread               thread_;   // declared last — initialized after others
};
