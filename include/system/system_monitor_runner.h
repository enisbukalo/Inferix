#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

namespace ftxui {
class ScreenInteractive;
}

/**
 * @file system_monitor_runner.h
 * @brief Manages a background thread that polls all monitors and triggers UI
 * redraws.
 *
 * This class implements a background monitoring thread that:
 * 1. Polls all hardware monitors (CPU, GPU, RAM) at regular intervals
 * 2. Triggers FTXUI screen redrawing after each poll
 * 3. Provides clean shutdown via stop() or destructor
 *
 * Thread model:
 * - Background thread: Calls update() on all monitors and posts redraw events
 * - Main thread: Receives redraw events and updates the UI
 * - Shared state: Protected by condition variable and mutex for coordination
 *
 * Timing:
 * - Polling interval: 250ms (kThreadWaitTimeMs)
 * - Effective update rate: ~4Hz (250ms per cycle)
 * - FTXUI redraw: Triggered after each poll cycle
 *
 * @note This class is not copyable or copy-assignable by design; monitor state
 *       and the running thread must remain uniquely owned.
 */
class SystemMonitorRunner
{
  public:
	/**
	 * @brief Constructs the runner and starts the background polling thread.
	 *
	 * This constructor:
	 * 1. Stores the reference to the FTXUI screen
	 * 2. Initializes the stop_flag_ to false
	 * 3. Creates and starts the background thread via thread_
	 *
	 * The background thread begins polling immediately after construction.
	 *
	 * @param screen Reference to the FTXUI interactive screen whose
	 *               PostEvent() will be called to trigger UI redraws.
	 * @note The screen must remain valid for the lifetime of this object.
	 * @note The background thread is started synchronously; it may begin
	 *       polling before this constructor returns.
	 */
	explicit SystemMonitorRunner(ftxui::ScreenInteractive &screen);

	/**
	 * @brief Deleted copy constructor — not copyable by design.
	 *
	 * This class manages a unique background thread and FTXUI screen
	 * reference that cannot be shared between instances.
	 */
	SystemMonitorRunner(const SystemMonitorRunner &) = delete;

	/**
	 * @brief Deleted copy-assignment operator — not copyable by design.
	 *
	 * This class manages a unique background thread and FTXUI screen
	 * reference that cannot be shared between instances.
	 */
	SystemMonitorRunner &operator=(const SystemMonitorRunner &) = delete;

	/**
	 * @brief Destructor. Calls @ref stop() to cleanly join the background
	 * thread.
	 *
	 * This destructor:
	 * 1. Sets stop_flag_ to true
	 * 2. Notifies cv_ to wake the polling thread
	 * 3. Joins the background thread
	 * 4. Ensures clean shutdown of all monitors
	 *
	 * @note Safe to call multiple times; subsequent calls are no-ops.
	 * @note The destructor blocks until the background thread has joined.
	 */
	~SystemMonitorRunner();

	/**
	 * @brief Sets the stop flag, wakes the polling thread, and joins it.
	 *
	 * This method:
	 * 1. Sets stop_flag_ to true
	 * 2. Notifies cv_ to wake the polling thread if it's waiting
	 * 3. Joins the background thread
	 * 4. Ensures clean shutdown of all monitors
	 *
	 * @note Safe to call multiple times; subsequent calls after the thread
	 *       has already been joined are no-ops.
	 * @note This method blocks until the background thread has joined.
	 * @see ~SystemMonitorRunner()
	 */
	void stop();

  private:
	/**
	 * @brief Background thread function that polls monitors and triggers redraws.
	 *
	 * This loop:
	 * 1. Calls update() on CpuMonitor, MemoryMonitor, and GpuMonitor
	 * 2. Posts a Custom event to the FTXUI screen to trigger redraw
	 * 3. Waits for kThreadWaitTimeMs (250ms) or until stop_flag_ is set
	 * 4. Repeats until stop_flag_ is true
	 *
	 * The loop uses a condition variable to allow the thread to wake up
	 * immediately if stop() is called, rather than waiting for the full
	 * polling interval.
	 *
	 * @note This method is called by the thread_ member.
	 * @note All monitor update() calls are thread-safe.
	 * @note The screen.PostEvent() call is safe from the background thread.
	 */
	void run();

	/**
	 * @brief Polling interval in milliseconds.
	 *
	 * The background thread waits this duration between poll cycles.
	 * A shorter interval provides more responsive UI updates but uses
	 * more CPU. A longer interval reduces CPU usage but may lag in
	 * reflecting system state changes.
	 *
	 * Value: 250ms (4 Hz update rate)
	 */
	const uint8_t kThreadWaitTimeMs = 250;

	/**
	 * @brief Reference to the FTXUI interactive screen.
	 *
	 * Used to post redraw events from the background thread.
	 * @note This reference must remain valid for the lifetime of
	 *       this object.
	 */
	ftxui::ScreenInteractive &screen_;

	/**
	 * @brief Stop flag for the background thread.
	 *
	 * When set to true, the run() loop terminates and joins.
	 * Protected by atomic operations for thread-safe access.
	 */
	std::atomic<bool> stop_flag_{ false };

	/**
	 * @brief Mutex for condition variable access.
	 *
	 * Protects access to cv_ during wait/notify operations.
	 */
	std::mutex cv_mutex_;

	/**
	 * @brief Condition variable for thread coordination.
	 *
	 * Used to wake the polling thread when stop() is called, allowing
	 * immediate shutdown rather than waiting for the full polling interval.
	 */
	std::condition_variable cv_;

	/**
	 * @brief Background thread for polling monitors.
	 *
	 * Declared last to ensure proper initialization order: stop_flag_,
	 * cv_mutex_, and cv_ must be initialized before thread_ starts.
	 */
	std::thread thread_;
};
