/**
 * @file pty_windows.cpp
 * @brief Windows-specific PTY implementation using ConPTY.
 *
 * Spawns a shell process via CreatePseudoConsole and provides
 * read/write access through named pipes.
 */

#include "pty_handler.h"

// ConPTY requires Windows 10 1809+; ensure the SDK version macros are set
// so that MinGW headers expose CreatePseudoConsole and friends.
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000006
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include <windows.h>

bool PtyHandler::spawn_windows(int cols, int rows)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (alive_)
		return false;

	HANDLE pipe_pty_in = INVALID_HANDLE_VALUE;
	HANDLE pipe_pty_out = INVALID_HANDLE_VALUE;
	HANDLE pipe_app_in = INVALID_HANDLE_VALUE;
	HANDLE pipe_app_out = INVALID_HANDLE_VALUE;

	// Create pipes: app writes to pipe_pty_in, reads from pipe_app_out
	if (!CreatePipe(&pipe_pty_in, &pipe_app_in, nullptr, 0))
		return false;

	if (!CreatePipe(&pipe_app_out, &pipe_pty_out, nullptr, 0)) {
		CloseHandle(pipe_pty_in);
		CloseHandle(pipe_app_in);
		return false;
	}

	// Create the pseudo console
	COORD size;
	size.X = static_cast<SHORT>(cols);
	size.Y = static_cast<SHORT>(rows);

	HPCON hpc = nullptr;
	HRESULT hr = CreatePseudoConsole(size, pipe_pty_in, pipe_pty_out, 0, &hpc);

	// Close the PTY-side pipe handles; ConPTY owns them now
	CloseHandle(pipe_pty_in);
	CloseHandle(pipe_pty_out);

	if (FAILED(hr)) {
		CloseHandle(pipe_app_in);
		CloseHandle(pipe_app_out);
		return false;
	}

	// Set up process attributes with the pseudo console
	SIZE_T attr_size = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &attr_size);

	auto attr_list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(
		HeapAlloc(GetProcessHeap(), 0, attr_size));

	if (!attr_list) {
		ClosePseudoConsole(hpc);
		CloseHandle(pipe_app_in);
		CloseHandle(pipe_app_out);
		return false;
	}

	if (!InitializeProcThreadAttributeList(attr_list, 1, 0, &attr_size)) {
		HeapFree(GetProcessHeap(), 0, attr_list);
		ClosePseudoConsole(hpc);
		CloseHandle(pipe_app_in);
		CloseHandle(pipe_app_out);
		return false;
	}

	if (!UpdateProcThreadAttribute(attr_list,
								   0,
								   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
								   hpc,
								   sizeof(HPCON),
								   nullptr,
								   nullptr)) {
		DeleteProcThreadAttributeList(attr_list);
		HeapFree(GetProcessHeap(), 0, attr_list);
		ClosePseudoConsole(hpc);
		CloseHandle(pipe_app_in);
		CloseHandle(pipe_app_out);
		return false;
	}

	// Launch the shell process
	STARTUPINFOEXW si = {};
	si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
	si.lpAttributeList = attr_list;

	PROCESS_INFORMATION pi = {};
	wchar_t cmd[] = L"cmd.exe";

	BOOL ok = CreateProcessW(nullptr,
							 cmd,
							 nullptr,
							 nullptr,
							 FALSE,
							 EXTENDED_STARTUPINFO_PRESENT,
							 nullptr,
							 nullptr,
							 &si.StartupInfo,
							 &pi);

	DeleteProcThreadAttributeList(attr_list);
	HeapFree(GetProcessHeap(), 0, attr_list);

	if (!ok) {
		ClosePseudoConsole(hpc);
		CloseHandle(pipe_app_in);
		CloseHandle(pipe_app_out);
		return false;
	}

	CloseHandle(pi.hThread);

	hpc_ = hpc;
	pipe_in_ = pipe_app_in;
	pipe_out_ = pipe_app_out;
	proc_handle_ = pi.hProcess;
	alive_ = true;
	return true;
}

int PtyHandler::read_windows(char *buf, std::size_t len)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || pipe_out_ == nullptr)
		return -1;

	DWORD available = 0;
	if (!PeekNamedPipe(static_cast<HANDLE>(pipe_out_),
					   nullptr,
					   0,
					   nullptr,
					   &available,
					   nullptr)) {
		return -1;
	}

	if (available == 0)
		return 0;

	DWORD to_read = (available < static_cast<DWORD>(len))
						? available
						: static_cast<DWORD>(len);
	DWORD bytes_read = 0;

	if (!ReadFile(static_cast<HANDLE>(pipe_out_),
				  buf,
				  to_read,
				  &bytes_read,
				  nullptr)) {
		return -1;
	}

	return static_cast<int>(bytes_read);
}

int PtyHandler::write_windows(const char *data, std::size_t len)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || pipe_in_ == nullptr)
		return -1;

	DWORD bytes_written = 0;
	if (!WriteFile(static_cast<HANDLE>(pipe_in_),
				   data,
				   static_cast<DWORD>(len),
				   &bytes_written,
				   nullptr)) {
		return -1;
	}

	return static_cast<int>(bytes_written);
}

bool PtyHandler::resize_windows(int cols, int rows)
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || hpc_ == nullptr)
		return false;

	COORD size;
	size.X = static_cast<SHORT>(cols);
	size.Y = static_cast<SHORT>(rows);

	HRESULT hr = ResizePseudoConsole(static_cast<HPCON>(hpc_), size);
	return SUCCEEDED(hr);
}

void PtyHandler::close_windows()
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_)
		return;

	if (hpc_) {
		ClosePseudoConsole(static_cast<HPCON>(hpc_));
		hpc_ = nullptr;
	}

	if (proc_handle_) {
		// Give the process a moment to exit gracefully
		if (WaitForSingleObject(static_cast<HANDLE>(proc_handle_), 500) !=
			WAIT_OBJECT_0) {
			TerminateProcess(static_cast<HANDLE>(proc_handle_), 1);
		}
		CloseHandle(static_cast<HANDLE>(proc_handle_));
		proc_handle_ = nullptr;
	}

	if (pipe_in_) {
		CloseHandle(static_cast<HANDLE>(pipe_in_));
		pipe_in_ = nullptr;
	}

	if (pipe_out_) {
		CloseHandle(static_cast<HANDLE>(pipe_out_));
		pipe_out_ = nullptr;
	}

	alive_ = false;
}

bool PtyHandler::is_alive_windows()
{
	std::lock_guard<std::mutex> lock(pty_mutex_);

	if (!alive_ || proc_handle_ == nullptr)
		return false;

	DWORD result = WaitForSingleObject(static_cast<HANDLE>(proc_handle_), 0);

	if (result == WAIT_OBJECT_0) {
		// Process has exited
		alive_ = false;
		return false;
	}

	return true;
}
