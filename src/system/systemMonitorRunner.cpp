/**
 * @file systemMonitorRunner.cpp
 * @brief Singleton background monitoring thread implementation.
 *
 * Implements the singleton polling thread that periodically updates all system
 * monitors (CPU, GPU, RAM) and triggers UI redraws via FTXUI events.
 * Supports dynamic refresh rate updates via EventBus subscription.
 */

#include "systemMonitorRunner.h"
#include "cpuMonitor.h"
#include "gpuMonitor.h"
#include "ramMonitor.h"
#include <iostream>

/**
 * @brief Private constructor for singleton.
 *
 * Initializes the singleton instance. Called only by instance().
 * The background thread is NOT started here; it's started by start().
 */
SystemMonitorRunner::SystemMonitorRunner()
{
}

/**
 * @brief Destructor. Calls stop() to cleanly shut down.
 *
 * Ensures the background thread is stopped and any active subscription
 * is cleaned up.
 */
SystemMonitorRunner::~SystemMonitorRunner()
{
	stop();
}

/**
 * @brief Get the singleton instance (Meyers' singleton).
 *
 * @return Reference to the singleton SystemMonitorRunner instance.
 */
SystemMonitorRunner& SystemMonitorRunner::instance()
{
	static SystemMonitorRunner instance;
	return instance;
}

/**
 * @brief Start the background polling thread.
 *
 * Initializes the singleton with the initial refresh rate, subscribes
 * to refresh rate change events, and starts the background polling thread.
 *
 * @param refreshRateMs Initial polling interval in milliseconds.
 *
 * @note Can only be called once; subsequent calls are ignored.
 * @note This method does NOT take a screen reference — FTXUI's Screen::Loop()
 *       handles all rendering independently. The background thread only updates
 *       monitor data.
 */
void SystemMonitorRunner::start(int refreshRateMs)
{
	// Ensure this runs exactly once
	std::call_once(startFlag_, [this, refreshRateMs]() {
		// Set initial refresh rate
		refreshRateMs_.store(refreshRateMs);
		
		// Subscribe to refresh rate change events
		subscriptionId_ = EventBus::subscribe(
			"config.ui.refreshRateMs",
			[this](const EventBus::EventId& event, const void* data) {
				this->onEvent(event, data);
			}
		);
		
		std::cout << "[SystemMonitorRunner] Started with refresh rate: "
			<< refreshRateMs << "ms, subscription ID: " << subscriptionId_ << std::endl;
		
		// Start the background polling thread
		thread_ = std::thread(&SystemMonitorRunner::run, this);
	});
}

/**
 * @brief Stop the background polling thread.
 *
 * Unsubscribes from EventBus, sets the stop flag, wakes the polling
 * thread, and joins it.
 *
 * @note Safe to call multiple times; subsequent calls are no-ops.
 */
void SystemMonitorRunner::stop()
{
	// Unsubscribe from EventBus if subscribed
	if (subscriptionId_ != 0) {
		EventBus::unsubscribe(subscriptionId_);
		subscriptionId_ = 0;
		std::cout << "[SystemMonitorRunner] Unsubscribed from EventBus" << std::endl;
	}
	
	// Signal the background thread to stop
	stopFlag_.store(true);
	cv_.notify_one();
	
	// Join the background thread
	if (thread_.joinable()) {
		thread_.join();
		std::cout << "[SystemMonitorRunner] Background thread joined" << std::endl;
	}
}

/**
 * @brief Event handler for EventBus.
 *
 * Handles "config.ui.refreshRateMs" events by updating the polling
 * interval dynamically.
 *
 * @param event Event identifier
 * @param data Pointer to the new refresh rate value (int*)
 */
void SystemMonitorRunner::onEvent(const EventBus::EventId& event, const void* data)
{
	if (event == "config.ui.refreshRateMs" && data != nullptr) {
		int newRate = *static_cast<const int*>(data);
		refreshRateMs_.store(newRate);
		std::cout << "[SystemMonitorRunner] Refresh rate updated to: "
			<< newRate << "ms" << std::endl;
	}
}

/**
 * @brief Background thread function that polls monitors.
 *
 * Polls all system monitors at the interval specified by refreshRateMs_,
 * which can be updated dynamically via EventBus.
 *
 * @note This method does NOT trigger UI redraws; FTXUI's Screen::Loop()
 *       handles all rendering independently.
 */
void SystemMonitorRunner::run()
{
	while (true) {
		{
			std::unique_lock<std::mutex> lock(cvMutex_);
			// Wait for the dynamic refresh interval or until stopped
			cv_.wait_for(lock,
					std::chrono::milliseconds(refreshRateMs_.load()),
					[this] { return stopFlag_.load(); });
		}
		if (stopFlag_.load())
			break;
		
		// Poll all monitors
		MemoryMonitor::instance().update();
		CpuMonitor::instance().update();
		GpuMonitor::instance().update();
	}
}
