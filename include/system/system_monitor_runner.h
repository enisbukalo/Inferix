#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ftxui {
class ScreenInteractive;
}

/**
 * @brief Manages a background thread that polls all monitors and triggers UI redraws.
 *
 * On construction the polling thread is started immediately. Every 500 ms the thread
 * calls each monitor's @c update() method and then signals the FTXUI screen to
 * redraw. The thread is stopped and joined either by calling @ref stop() explicitly
 * or automatically by the destructor.
 *
 * @note This class is not copyable or copy-assignable by design; monitor state and
 *       the running thread must remain uniquely owned.
 */
class SystemMonitorRunner {
  public:
	/**
	 * @brief Constructs the runner and starts the background polling thread.
	 *
	 * @param screen The FTXUI interactive screen whose @c PostEvent will be called
	 *               to trigger redraws.
	 */
	explicit SystemMonitorRunner(ftxui::ScreenInteractive &screen);

	/**
	 * @brief Deleted copy constructor — not copyable by design.
	 */
	SystemMonitorRunner(const SystemMonitorRunner &) = delete;

	/**
	 * @brief Deleted copy-assignment operator — not copyable by design.
	 */
	SystemMonitorRunner &operator=(const SystemMonitorRunner &) = delete;

	/**
	 * @brief Destructor. Calls @ref stop() to cleanly join the background thread.
	 */
	~SystemMonitorRunner();

	/**
	 * @brief Sets the stop flag, wakes the polling thread, and joins it.
	 *
	 * Safe to call multiple times; subsequent calls after the thread has already
	 * been joined are no-ops.
	 */
	void stop();

  private:
	void run();

	ftxui::ScreenInteractive &screen_;
	std::atomic<bool> stop_flag_{false};
	std::mutex cv_mutex_;
	std::condition_variable cv_;
	std::thread thread_; // declared last — initialized after others
};
