#pragma once
#include <string>

namespace Config {

/**
 * @file ui_settings.h
 * @brief UI configuration settings for the Inferix terminal interface.
 *
 * This header defines the UISettings structure which controls the
 * appearance and behavior of the terminal user interface.
 */

namespace Config {

/**
 * @brief UI/settings panel settings.
 *
 * Controls the appearance and behavior of the terminal UI:
 * - **Theme**: Visual theme selection for colors and styling
 * - **Default tab**: Which tab opens on startup
 * - **Show system panel**: Toggle system monitor visibility
 * - **Refresh rate**: How often to update monitoring data
 *
 * @note The default_tab values correspond to tab indices in the
 *       main UI window. The mapping may change as new tabs are added.
 *
 * @code
 * // Configure UI for development
 * UISettings ui;
 * ui.theme = "dark";
 * ui.default_tab = 1;  // Open Server Log by default
 * ui.refresh_rate_ms = 100;  // Fast updates for monitoring
 * @endcode
 */
struct UISettings
{
	/**
	 * @brief Visual theme name.
	 *
	 * Determines colors, fonts, and styling throughout the UI.
	 * Available themes depend on the theme system implementation.
	 *
	 * @default "default"
	 * @note Common theme names: "default", "dark", "light", "monokai"
	 */
	std::string theme = "default";

	/**
	 * @brief Default tab to show on startup.
	 *
	 * The tab index that opens when the application starts:
	 * - 0: Settings tab
	 * - 1: Server Log tab
	 * - 2: Terminal tab
	 *
	 * Corresponds to: Tab index in main UI window
	 * @default 0 (Settings)
	 * @note Tab indices may change as new tabs are added to the UI.
	 */
	int default_tab = 0;

	/**
	 * @brief Show system monitoring panel.
	 *
	 * When enabled, displays real-time system metrics including:
	 * - CPU usage
	 * - Memory usage
	 * - GPU utilization (if available)
	 * - Network activity
	 *
	 * @default true
	 * @note Disabling may improve performance on low-end systems.
	 */
	bool show_system_panel = true;

	/**
	 * @brief Refresh rate for monitoring data in milliseconds.
	 *
	 * How frequently the system panel and other dynamic elements
	 * update. Lower values = smoother but more CPU intensive.
	 *
	 * @default 250 (4 times per second)
	 * @range 50-1000 recommended
	 * @note Values below 100ms may cause noticeable CPU usage.
	 */
	int refresh_rate_ms = 250;
};

} // namespace Config

} // namespace Config
