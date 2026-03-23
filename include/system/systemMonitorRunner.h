#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include "eventBus.h"

namespace ftxui {
class ScreenInteractive;
}

/**
 * @file systemMonitorRunner.h
 * @brief Singleton that manages a background thread polling all monitors and
 * triggering UI redraws.
 *
 * This class implements a background monitoring thread that:
 * 1. Polls all hardware monitors (CPU, GPU, RAM) at configurable intervals
 * 2. Triggers FTXUI screen redrawing after each poll
 * 3. Dynamically updates polling interval via EventBus subscription
 * 4. Provides clean shutdown via stop() or destructor
 *
 * Thread model:
 * - Background thread: Calls update() on all monitors and posts redraw events
 * - Main thread: Receives redraw events and updates the UI
 * - Shared state: Protected by condition variable and mutex for coordination
 *
 * Timing:
 * - Polling interval: Configurable via refreshRateMs_ (default 250ms)
 * - Effective update rate: Dynamic, updated via "config.ui.refreshRateMs" event
 * - FTXUI redraw: Triggered after each poll cycle
 *
 * Singleton lifecycle:
 * - Created: First call to instance()
 * - Started: start(screen, refreshRateMs) called once from main()
 * - Stopped: stop() called explicitly or via destructor
 *
 * @note This class follows Meyers' singleton pattern — thread-safe, lazy
 *       initialization, automatic cleanup at program termination.
 * @note start() can only be called once; subsequent calls are ignored.
 */
class SystemMonitorRunner
{
  public:
	/**
	 * @brief Get the singleton instance.
	 *
	 * Creates the instance on first call using Meyers' singleton pattern.
	 * Thread-safe in C++11 and later.
	 *
	 * @return Reference to the singleton SystemMonitorRunner instance.
	 *
	 * @code
	 * // In main.cpp:
	 * SystemMonitorRunner::instance().start(screen, config.ui.refreshRateMs);
	 * @endcode
	 *
	 * @note The instance is destroyed at program termination.
	 * @note This method is thread-safe; multiple simultaneous calls return
	 *       the same instance.
	 */
	static SystemMonitorRunner& instance();

	/**
	 * @brief Start the background polling thread.
	 *
	 * Initializes the singleton with the FTXUI screen reference and initial
	 * refresh rate, then starts the background polling thread. Also subscribes
	 * to "config.ui.refreshRateMs" events for dynamic updates.
	 *
	 * This method:
	 * 1. Stores the screen reference
	 * 2. Sets the initial refresh rate
	 * 3. Subscribes to refresh rate change events
	 * 4. Starts the background polling thread
	 *
	 * @param screen Reference to the FTXUI interactive screen.
	 * @param refreshRateMs Initial polling interval in milliseconds.
	 *
	 * @note Can only be called once; subsequent calls are ignored.
	 * @note The screen must remain valid for the lifetime of this object.
	 * @note The background thread starts polling immediately.
	 *
	 * @code
	 * // In main.cpp:
	 * ConfigManager::instance().load();
	 * auto& config = ConfigManager::instance().getConfig();
	 * SystemMonitorRunner::instance().start(screen, config.ui.refreshRateMs);
	 * @endcode
	 */
	void start(ftxui::ScreenInteractive& screen, int refreshRateMs);

	/**
	 * @brief Stop the background polling thread.
	 *
	 * This method:
	 * 1. Unsubscribes from EventBus
	 * 2. Sets stopFlag_ to true
	 * 3. Notifies cv_ to wake the polling thread
	 * 4. Joins the background thread
	 *
	 * @note Safe to call multiple times; subsequent calls are no-ops.
	 * @note Blocks until the background thread has joined.
	 * @note Should be called from main() before exit for clean shutdown.
	 *
	 * @code
	 * // In main.cpp:
	 * App::run();
	 * SystemMonitorRunner::instance().stop();
	 * return 0;
	 * @endcode
	 */
	void stop();

	/**
	 * @brief Deleted copy constructor — not copyable by design.
	 *
	 * Singleton cannot be copied.
	 */
	SystemMonitorRunner(const SystemMonitorRunner&) = delete;

	/**
	 * @brief Deleted copy-assignment operator — not copyable by design.
	 *
	 * Singleton cannot be copied.
	 */
	SystemMonitorRunner& operator=(const SystemMonitorRunner&) = delete;

  private:
	/**
	 * @brief Private constructor for singleton.
	 *
	 * Initializes the singleton instance. Called only by instance().
	 */
	SystemMonitorRunner();

	/**
	 * @brief Destructor. Calls @ref stop() to cleanly join the background
	 * thread.
	 *
	 * This destructor:
	 * 1. Calls stop() to terminate the background thread
	 * 2. Unsubscribes from EventBus (if still subscribed)
	 * 3. Ensures clean shutdown
	 *
	 * @note Safe to call multiple times; subsequent calls are no-ops.
	 * @note The destructor blocks until the background thread has joined.
	 */
	~SystemMonitorRunner();

	/**
	 * @brief Event handler for EventBus.
	 *
	 * Called when a subscribed event is published. Currently handles:
	 * - "config.ui.refreshRateMs": Updates the polling interval dynamically
	 *
	 * @param event Event identifier
	 * @param data Pointer to event data (e.g., new refresh rate value)
	 *
	 * @note Called from the thread that publishes the event (typically UI thread).
	 * @note Uses atomic store for lock-free refresh rate updates.
	 *
	 * @code
	 * // In SettingsPanel::saveConfig():
	 * if (oldRefreshRate != newRefreshRate) {
	 *     EventBus::publish("config.ui.refreshRateMs", &newRefreshRate);
	 * }
	 * @endcode
	 */
	void onEvent(const EventBus::EventId& event, const void* data);

	/**
	 * @brief Background thread function that polls monitors and triggers
	 * redraws.
	 *
	 * This loop:
	 * 1. Calls update() on CpuMonitor, MemoryMonitor, and GpuMonitor
	 * 2. Posts a Custom event to the FTXUI screen to trigger redraw
	 * 3. Waits for refreshRateMs_ or until stopFlag_ is set
	 * 4. Repeats until stopFlag_ is true
	 *
	 * The loop uses a condition variable to allow the thread to wake up
	 * immediately if stop() is called, rather than waiting for the full
	 * polling interval.
	 *
	 * @note This method is called by the thread_ member.
	 * @note All monitor update() calls are thread-safe.
	 * @note The screen.PostEvent() call is safe from the background thread.
	 * @note Uses dynamic refreshRateMs_ instead of constant kThreadWaitTimeMs.
	 */
	void run();

	/**
	 * @brief Reference to the FTXUI interactive screen.
	 *
	 * Used to post redraw events from the background thread.
	 * @note This reference must remain valid for the lifetime of
	 *       this object.
	 * @note Set by start() method.
	 */
	ftxui::ScreenInteractive* screen_ = nullptr;

	/**
	 * @brief Dynamic polling interval in milliseconds.
	 *
	 * The background thread waits this duration between poll cycles.
	 * Updated dynamically via EventBus subscription to
	 * "config.ui.refreshRateMs" events.
	 *
	 * @note Uses std::atomic for lock-free reads/writes.
	 * @note Default value: 250ms (4 Hz update rate)
	 * @note Set by start() and updated by onEvent().
	 */
	std::atomic<int> refreshRateMs_{ 250 };

	/**
	 * @brief Stop flag for the background thread.
	 *
	 * When set to true, the run() loop terminates and joins.
	 * Protected by atomic operations for thread-safe access.
	 */
	std::atomic<bool> stopFlag_{ false };

	/**
	 * @brief Mutex for condition variable access.
	 *
	 * Protects access to cv_ during wait/notify operations.
	 */
	std::mutex cvMutex_;

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
	 * Declared last to ensure proper initialization order: stopFlag_,
	 * cvMutex_, and cv_ must be initialized before thread_ starts.
	 */
	std::thread thread_;

	/**
	 * @brief Ensures start() can only be called once.
	 *
	 * Used with std::call_once to guarantee the background thread
	 * is started exactly once, even if start() is called multiple times.
	 *
	 * @note This provides thread-safe one-time initialization.
	 */
	std::once_flag startFlag_;

	/**
	 * @brief EventBus subscription ID for refresh rate changes.
	 *
	 * Used to unsubscribe when stop() is called or in destructor.
	 * Subscribes to "config.ui.refreshRateMs" events.
	 *
	 * @note Initialized to 0 (invalid subscription).
	 * @note Set by start(), cleared by stop().
	 */
	EventBus::SubscriptionId subscriptionId_ = 0;
};
