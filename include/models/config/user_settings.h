#pragma once
#include <string>

namespace Config {

/**
 * @file user_settings.h
 * @brief Terminal emulator configuration settings.
 *
 * This header defines the TerminalSettings structure which controls
 * the embedded terminal emulator's behavior and initialization.
 */

namespace Config {

/**
 * @brief Terminal emulator settings.
 *
 * Configuration for the embedded terminal emulator:
 * - **Default shell**: Override system default shell
 * - **Initial command**: Command to run after shell starts
 * - **Working directory**: Starting directory for the shell
 * - **Default dimensions**: Fallback terminal size
 *
 * The terminal emulator provides an interactive shell within the
 * Inferix application, useful for debugging, running commands,
 * or accessing the system directly.
 *
 * @note Empty string values typically mean "use system default"
 *       or "not configured".
 *
 * @code
 * // Configure terminal for development
 * TerminalSettings terminal;
 * terminal.default_shell = "/bin/bash";
 * terminal.working_directory = "/home/user/projects";
 * terminal.initial_command = "git status";
 * @endcode
 */
struct TerminalSettings
{
	/**
	 * @brief Shell program to execute.
	 *
	 * The shell executable to use. If empty, uses the system default:
	 * - Windows: cmd.exe or PowerShell
	 * - Linux/macOS: /bin/bash or user's configured shell
	 *
	 * @default "" (system default)
	 * @note Examples: "/bin/bash", "/bin/zsh", "powershell.exe"
	 * @see std::system for shell execution details
	 */
	std::string default_shell;

	/**
	 * @brief Command to execute after shell starts.
	 *
	 * A command that runs automatically when the terminal opens.
	 * Useful for setting up environment or running diagnostics.
	 *
	 * @default "" (none)
	 * @note The command runs in the context of default_shell.
	 * @note Examples: "cd /project && git status", "nvim"
	 */
	std::string initial_command;

	/**
	 * @brief Working directory for the shell.
	 *
	 * The directory the shell starts in. If empty or invalid,
	 * uses the application's current working directory.
	 *
	 * @default "" (current directory)
	 * @note Should be an absolute path for reliability.
	 * @note The directory must exist and be accessible.
	 */
	std::string working_directory;

	/**
	 * @brief Default terminal width in columns.
	 *
	 * Fallback column count when the terminal size cannot be
	 * determined or for initial allocation.
	 *
	 * @default 80
	 * @range 16 to maximum terminal width
	 * @note Modern terminals typically support 120+ columns.
	 */
	int default_cols = 80;

	/**
	 * @brief Default terminal height in rows.
	 *
	 * Fallback row count when the terminal size cannot be
	 * determined or for initial allocation.
	 *
	 * @default 24
	 * @range 8 to maximum terminal height
	 * @note Combined with default_cols, determines initial buffer size.
	 */
	int default_rows = 24;
};

} // namespace Config

} // namespace Config
