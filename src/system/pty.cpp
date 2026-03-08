/**
 * @file pty.cpp
 * @brief PTY handler platform dispatcher.
 *
 * Routes each public method to the appropriate Linux, Windows, or
 * unknown-platform implementation based on compile-time definitions.
 */

#include "pty_handler.h"

bool PtyHandler::spawn(int cols, int rows)
{
#ifdef _WIN32
	return spawn_windows(cols, rows);
#elif __linux__
	return spawn_linux(cols, rows);
#else
	return spawn_unknown(cols, rows);
#endif
}

int PtyHandler::read(char *buf, std::size_t len)
{
#ifdef _WIN32
	return read_windows(buf, len);
#elif __linux__
	return read_linux(buf, len);
#else
	return read_unknown(buf, len);
#endif
}

int PtyHandler::write(const char *data, std::size_t len)
{
#ifdef _WIN32
	return write_windows(data, len);
#elif __linux__
	return write_linux(data, len);
#else
	return write_unknown(data, len);
#endif
}

bool PtyHandler::resize(int cols, int rows)
{
#ifdef _WIN32
	return resize_windows(cols, rows);
#elif __linux__
	return resize_linux(cols, rows);
#else
	return resize_unknown(cols, rows);
#endif
}

void PtyHandler::close()
{
#ifdef _WIN32
	close_windows();
#elif __linux__
	close_linux();
#else
	close_unknown();
#endif
}

bool PtyHandler::is_alive()
{
#ifdef _WIN32
	return is_alive_windows();
#elif __linux__
	return is_alive_linux();
#else
	return is_alive_unknown();
#endif
}

PtyHandler::~PtyHandler()
{
	close();
}

// Unknown platform stubs
bool PtyHandler::spawn_unknown([[maybe_unused]] int cols,
							   [[maybe_unused]] int rows)
{
	return false;
}

int PtyHandler::read_unknown([[maybe_unused]] char *buf,
							 [[maybe_unused]] std::size_t len)
{
	return -1;
}

int PtyHandler::write_unknown([[maybe_unused]] const char *data,
							  [[maybe_unused]] std::size_t len)
{
	return -1;
}

bool PtyHandler::resize_unknown([[maybe_unused]] int cols,
								[[maybe_unused]] int rows)
{
	return false;
}

void PtyHandler::close_unknown()
{
}

bool PtyHandler::is_alive_unknown()
{
	return false;
}
