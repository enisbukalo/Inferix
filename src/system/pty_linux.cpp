/**
 * @file pty_linux.cpp
 * @brief Linux-specific PTY implementation using forkpty().
 *
 * Spawns a shell process via forkpty and provides non-blocking read/write
 * access to the master side of the pseudo-terminal.
 */

#include "pty_handler.h"

#include <cstdlib>
#include <fcntl.h>
#include <pty.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

bool PtyHandler::spawn_linux(int cols, int rows)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (alive_)
		return false;

	struct winsize ws = {};
	ws.ws_col = static_cast<unsigned short>(cols);
	ws.ws_row = static_cast<unsigned short>(rows);

	int master = -1;
	pid_t pid = forkpty(&master, nullptr, nullptr, &ws);

	if (pid < 0) {
		return false;
	}

	if (pid == 0) {
		// Child process: exec a shell
		const char *shell = std::getenv("SHELL");
		if (!shell)
			shell = "/bin/sh";
		execlp(shell, shell, nullptr);
		_exit(127);
	}

	// Parent: set master fd to non-blocking
	int flags = fcntl(master, F_GETFL, 0);
	if (flags != -1)
		fcntl(master, F_SETFL, flags | O_NONBLOCK);

	master_fd_ = master;
	child_pid_ = pid;
	alive_ = true;
	return true;
}

int PtyHandler::read_linux(char *buf, std::size_t len)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || master_fd_ < 0)
		return -1;

	ssize_t n = ::read(master_fd_, buf, len);
	if (n < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;
		return -1;
	}
	return static_cast<int>(n);
}

int PtyHandler::write_linux(const char *data, std::size_t len)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || master_fd_ < 0)
		return -1;

	ssize_t n = ::write(master_fd_, data, len);
	return (n < 0) ? -1 : static_cast<int>(n);
}

bool PtyHandler::resize_linux(int cols, int rows)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || master_fd_ < 0)
		return false;

	struct winsize ws = {};
	ws.ws_col = static_cast<unsigned short>(cols);
	ws.ws_row = static_cast<unsigned short>(rows);

	return ioctl(master_fd_, TIOCSWINSZ, &ws) == 0;
}

void PtyHandler::close_linux()
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_)
		return;

	if (master_fd_ >= 0) {
		::close(master_fd_);
		master_fd_ = -1;
	}

	if (child_pid_ > 0) {
		int status;
		waitpid(child_pid_, &status, 0);
		child_pid_ = -1;
	}

	alive_ = false;
}

bool PtyHandler::is_alive_linux()
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || child_pid_ <= 0)
		return false;

	int status;
	pid_t result = waitpid(child_pid_, &status, WNOHANG);

	if (result == 0) {
		// Child still running
		return true;
	}

	// Child has exited
	alive_ = false;
	if (master_fd_ >= 0) {
		::close(master_fd_);
		master_fd_ = -1;
	}
	child_pid_ = -1;
	return false;
}
