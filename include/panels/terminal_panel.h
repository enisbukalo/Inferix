/**
 * @file terminal_panel.h
 * @brief Stateful terminal panel that embeds a PTY + VTerm in an FTXUI tab.
 *
 * Spawns a host shell via PtyHandler, feeds PTY output through libvterm
 * for ANSI parsing, and renders the virtual terminal screen as FTXUI
 * elements. Returns an ftxui::Component that captures keyboard events.
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "pty_handler.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>

struct VTerm;
struct VTermScreen;

class TerminalPanel
{
  public:
	explicit TerminalPanel(ftxui::ScreenInteractive &screen,
						   std::string initial_command = "");
	~TerminalPanel();

	TerminalPanel(const TerminalPanel &) = delete;
	TerminalPanel &operator=(const TerminalPanel &) = delete;

	ftxui::Component Component();
	void Spawn();
	void Shutdown();
	bool IsSpawned() const;
	bool WantsEvent(ftxui::Event event) const;
	bool IsCapturing() const;

	bool HandleEvent(ftxui::Event event);

	friend void Vterm_output_cb(const char *s, size_t len, void *user);

  private:
	void ReadLoop();
	void Resize(int new_cols, int new_rows);
	ftxui::Element RenderScreen();

	ftxui::ScreenInteractive &screen_;
	VTerm *vt_ = nullptr;
	VTermScreen *vts_ = nullptr;
	int rows_ = 24;
	int cols_ = 80;
	std::atomic<bool> stop_flag_{ false };
	std::atomic<bool> spawned_{ false };
	std::atomic<bool> pty_dead_{ false };
	std::mutex vterm_mutex_;
	std::mutex cv_mutex_;
	std::condition_variable cv_;
	std::thread read_thread_;
	ftxui::Box box_ = {};
	std::string initial_command_;
	std::atomic<bool> initial_cmd_sent_{ false };
	std::atomic<bool> capturing_{ true };
	PtyHandler pty_;
};
