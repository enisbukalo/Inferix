#pragma once
#include <cstddef>
#include <mutex>

/**
 * @brief Thread-safe singleton that manages a pseudo-terminal (PTY).
 *
 * Spawns a shell process via a platform-specific PTY backend (ConPTY on
 * Windows, forkpty on Linux) and provides read/write/resize operations.
 * All public methods are safe to call from multiple threads concurrently.
 */
class PtyHandler
{
  public:
	static PtyHandler &instance()
	{
		static PtyHandler handler;
		return handler;
	}

	/**
	 * @brief Spawns a new shell process in a PTY of the given size.
	 * @return true on success, false on failure.
	 */
	bool spawn(int cols, int rows);

	/**
	 * @brief Reads available output from the PTY into @p buf.
	 * @return Number of bytes read, 0 if nothing available, or -1 on error.
	 */
	int read(char *buf, std::size_t len);

	/**
	 * @brief Writes @p len bytes of @p data to the PTY.
	 * @return Number of bytes written, or -1 on error.
	 */
	int write(const char *data, std::size_t len);

	/**
	 * @brief Resizes the PTY to the given dimensions.
	 * @return true on success.
	 */
	bool resize(int cols, int rows);

	/**
	 * @brief Closes the PTY and terminates the child process.
	 */
	void close();

	/**
	 * @brief Checks whether the child process is still running.
	 */
	bool is_alive();

	~PtyHandler();

  private:
	PtyHandler() = default;
	PtyHandler(const PtyHandler &) = delete;
	PtyHandler &operator=(const PtyHandler &) = delete;

	mutable std::mutex pty_mutex_;
	bool alive_ = false;

	// Platform-specific dispatch targets
	bool spawn_linux(int cols, int rows);
	bool spawn_windows(int cols, int rows);
	bool spawn_unknown(int cols, int rows);

	int read_linux(char *buf, std::size_t len);
	int read_windows(char *buf, std::size_t len);
	int read_unknown(char *buf, std::size_t len);

	int write_linux(const char *data, std::size_t len);
	int write_windows(const char *data, std::size_t len);
	int write_unknown(const char *data, std::size_t len);

	bool resize_linux(int cols, int rows);
	bool resize_windows(int cols, int rows);
	bool resize_unknown(int cols, int rows);

	void close_linux();
	void close_windows();
	void close_unknown();

	bool is_alive_linux();
	bool is_alive_windows();
	bool is_alive_unknown();

#ifdef _WIN32
	void *hpc_ = nullptr;
	void *pipe_in_ = nullptr;
	void *pipe_out_ = nullptr;
	void *proc_handle_ = nullptr;
#else
	int master_fd_ = -1;
	int child_pid_ = -1;
#endif
};
