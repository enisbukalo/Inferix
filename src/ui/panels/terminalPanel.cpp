/**
 * @file terminalPanel.cpp
 * @brief Terminal panel implementation — PTY + VTerm + FTXUI rendering.
 *
 * Wires together PtyHandler (platform PTY), libvterm (ANSI parser), and
 * FTXUI (TUI rendering) to provide an embedded terminal emulator inside
 * the Inferix application.
 */

#include "terminalPanel.h"

#include <vterm.h>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <cstring>
#include <string>
#include <vector>

using namespace ftxui;

// ---------------------------------------------------------------------------
// UTF-8 encoding helper
// ---------------------------------------------------------------------------

static std::string Codepoint_to_utf8(uint32_t cp)
{
	std::string out;
	if (cp < 0x80) {
		out += static_cast<char>(cp);
	} else if (cp < 0x800) {
		out += static_cast<char>(0xC0 | (cp >> 6));
		out += static_cast<char>(0x80 | (cp & 0x3F));
	} else if (cp < 0x10000) {
		out += static_cast<char>(0xE0 | (cp >> 12));
		out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (cp & 0x3F));
	} else if (cp < 0x110000) {
		out += static_cast<char>(0xF0 | (cp >> 18));
		out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
		out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (cp & 0x3F));
	}
	return out;
}

// ---------------------------------------------------------------------------
// vterm output callback — keypresses flow through vterm → PTY
// ---------------------------------------------------------------------------

void Vterm_output_cb(const char *s, size_t len, void *user)
{
	auto *panel = static_cast<TerminalPanel *>(user);
	panel->pty_.write(s, len);
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

TerminalPanel::TerminalPanel(ScreenInteractive &screen,
							 std::string initialCommand)
	: screen_(screen), initialCommand_(std::move(initialCommand))
{
}

TerminalPanel::~TerminalPanel()
{
	Shutdown();
}

bool TerminalPanel::IsSpawned() const
{
	return spawned_.load();
}

// ---------------------------------------------------------------------------
// Spawn — create vterm, start PTY, launch read thread
// ---------------------------------------------------------------------------

void TerminalPanel::Spawn()
{
	if (spawned_.load())
		return;

	// Use reflected box dimensions if available, otherwise keep defaults.
	int boxCols = (box_.x_max - box_.x_min + 1) - 2;
	int boxRows = (box_.y_max - box_.y_min + 1) - 2;
	if (boxCols > 0 && boxRows > 0) {
		cols_ = boxCols;
		rows_ = boxRows;
	}

	vt_ = vterm_new(rows_, cols_);
	vterm_set_utf8(vt_, 1);

	vts_ = vterm_obtain_screen(vt_);
	vterm_screen_reset(vts_, 1);

	vterm_output_set_callback(vt_, Vterm_output_cb, this);

	if (!pty_.spawn(cols_, rows_)) {
		vterm_free(vt_);
		vt_ = nullptr;
		vts_ = nullptr;
		return;
	}

	stopFlag_.store(false);
	ptyDead_.store(false);
	spawned_.store(true);

	initialCmdSent_.store(false);
	readThread_ = std::thread(&TerminalPanel::ReadLoop, this);
}

// ---------------------------------------------------------------------------
// Shutdown — stop thread, close PTY, free vterm
// ---------------------------------------------------------------------------

void TerminalPanel::Shutdown()
{
	if (!spawned_.load())
		return;

	stopFlag_.store(true);
	cv_.notify_one();

	if (readThread_.joinable())
		readThread_.join();

	pty_.close();

	if (vt_) {
		vterm_free(vt_);
		vt_ = nullptr;
		vts_ = nullptr;
	}

	spawned_.store(false);
}

// ---------------------------------------------------------------------------
// ReadLoop — background thread: PTY → vterm → redraw
// ---------------------------------------------------------------------------

void TerminalPanel::ReadLoop()
{
	char buf[4096];

	while (!stopFlag_.load()) {
		int n = pty_.read(buf, sizeof(buf));

		if (n > 0) {
			std::lock_guard<std::mutex> lock(vtermMutex_);
			vterm_input_write(vt_, buf, static_cast<size_t>(n));
			screen_.PostEvent(Event::Custom);

			// Send the initial command after the shell produces its first
			// output (prompt), so it is ready to accept input.
			if (!initialCommand_.empty() && !initialCmdSent_.load()) {
				initialCmdSent_.store(true);
				std::string cmd = initialCommand_ + "\r";
				pty_.write(cmd.data(), cmd.size());
			}
		} else if (n == 0) {
			// No data available — sleep briefly
			std::unique_lock<std::mutex> lock(cvMutex_);
			cv_.wait_for(lock, std::chrono::milliseconds(16), [this] {
				return stopFlag_.load();
			});
		} else {
			// PTY error / closed
			if (!pty_.isAlive()) {
				ptyDead_.store(true);
				screen_.PostEvent(Event::Custom);
				break;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Resize — update vterm + PTY dimensions (caller must hold vtermMutex_)
// ---------------------------------------------------------------------------

void TerminalPanel::Resize(int newCols, int newRows)
{
	vterm_set_size(vt_, newRows, newCols);
	pty_.resize(newCols, newRows);
	cols_ = newCols;
	rows_ = newRows;
}

// ---------------------------------------------------------------------------
// RenderScreen — read vterm cells → FTXUI elements
// ---------------------------------------------------------------------------

Element TerminalPanel::RenderScreen()
{
	if (!spawned_.load()) {
		return window(text(""),
					  center(text("Terminal not started.") | dim),
					  EMPTY) |
			   flex;
	}

	if (ptyDead_.load()) {
		return window(text(""),
					  center(text("Shell process has exited.") | dim),
					  EMPTY) |
			   flex;
	}

	std::lock_guard<std::mutex> lock(vtermMutex_);

	// Resize vterm + PTY if the container dimensions changed.
	int newCols = (box_.x_max - box_.x_min + 1) - 2; // subtract window border
	int newRows = (box_.y_max - box_.y_min + 1) - 2;
	if (newCols < 1)
		newCols = 1;
	if (newRows < 1)
		newRows = 1;
	if (newCols > 0 && newRows > 0 && box_.x_max > 0 && box_.y_max > 0 &&
		(newCols != cols_ || newRows != rows_)) {
		Resize(newCols, newRows);
	}

	// Query cursor position
	VTermPos cursorPos = {};
	vterm_state_get_cursorpos(vterm_obtain_state(vt_), &cursorPos);

	Elements lines;
	lines.reserve(static_cast<size_t>(rows_));

	for (int row = 0; row < rows_; ++row) {
		Elements cells;
		cells.reserve(static_cast<size_t>(cols_));

		for (int col = 0; col < cols_;) {
			VTermPos pos;
			pos.row = row;
			pos.col = col;

			VTermScreenCell cell;
			vterm_screen_get_cell(vts_, pos, &cell);

			// Build character string from codepoints
			std::string ch;
			if (cell.chars[0] == 0) {
				ch = " ";
			} else {
				for (int i = 0;
					 i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i] != 0;
					 ++i) {
					ch += Codepoint_to_utf8(cell.chars[i]);
				}
			}

			auto elem = text(ch);

			// Apply foreground colour
			VTermColor fg = cell.fg;
			vterm_screen_convert_color_to_rgb(vts_, &fg);
			if (!VTERM_COLOR_IS_DEFAULT_FG(&fg)) {
				elem = elem |
					   color(Color::RGB(fg.rgb.red, fg.rgb.green, fg.rgb.blue));
			}

			// Apply background colour
			VTermColor bg = cell.bg;
			vterm_screen_convert_color_to_rgb(vts_, &bg);
			if (!VTERM_COLOR_IS_DEFAULT_BG(&bg)) {
				elem =
					elem |
					bgcolor(Color::RGB(bg.rgb.red, bg.rgb.green, bg.rgb.blue));
			}

			// Apply attributes
			if (cell.attrs.bold)
				elem = elem | bold;
			if (cell.attrs.underline)
				elem = elem | underlined;
			if (cell.attrs.reverse)
				elem = elem | inverted;
			if (cell.attrs.italic)
				elem = elem | dim; // FTXUI has no italic; use dim as fallback

			// Apply cursor inversion if this cell is at cursor position
			if (cursorPos.row == row && cursorPos.col == col)
				elem = elem | inverted;

			cells.push_back(elem);

			// Advance by cell width (wide chars occupy 2 columns)
			col += (cell.width > 1) ? cell.width : 1;
		}

		lines.push_back(hbox(std::move(cells)));
	}

	auto content = vbox(std::move(lines));
	if (!capturing_.load()) {
		auto hint =
			text(" DETACHED - Press Ctrl+T to re-attach ") | bold | inverted;
		content = vbox({ content, hint });
	}
	return window(text(""), content, EMPTY) | flex;
}

// ---------------------------------------------------------------------------
// HandleEvent — keyboard input → vterm → PTY
// ---------------------------------------------------------------------------

bool TerminalPanel::IsCapturing() const
{
	return capturing_.load();
}

void TerminalPanel::SetCapturing(bool value)
{
	capturing_.store(value);
}

bool TerminalPanel::HandleEvent(Event event)
{
	if (!spawned_.load() || ptyDead_.load())
		return false;

	// Ctrl+T toggles capture mode — releases input to the UI.
	// Check both raw input byte and FTXUI character representation.
	const std::string &raw = event.input();
	if (raw.size() == 1 && raw[0] == '\x14') {
		capturing_.store(!capturing_.load());
		screen_.PostEvent(Event::Custom); // trigger redraw for hint
		return true;
	}

	// When not capturing, only Ctrl+T is handled (above). Let everything
	// else fall through to FTXUI for tab navigation, etc.
	if (!capturing_.load())
		return false;

	std::lock_guard<std::mutex> lock(vtermMutex_);

	VTermModifier mod = VTERM_MOD_NONE;

	// Ctrl+J sends a literal newline (LF) without executing - useful for
	// multi-line commands. Check raw input since FTXUI normalizes Ctrl+J
	// to Event::Return, same as Enter key.
	if (raw.size() == 1 && raw[0] == '\x0a') {
		vterm_keyboard_unichar(vt_, '\n', mod);
		return true;
	}

	// Special keys
	if (event == Event::Return) {
		vterm_keyboard_key(vt_, VTERM_KEY_ENTER, mod);
		return true;
	}
	if (event == Event::Tab) {
		vterm_keyboard_key(vt_, VTERM_KEY_TAB, mod);
		return true;
	}
	if (event == Event::Backspace) {
		vterm_keyboard_key(vt_, VTERM_KEY_BACKSPACE, mod);
		return true;
	}
	if (event == Event::Escape) {
		vterm_keyboard_key(vt_, VTERM_KEY_ESCAPE, mod);
		return true;
	}
	if (event == Event::ArrowUp) {
		vterm_keyboard_key(vt_, VTERM_KEY_UP, mod);
		return true;
	}
	if (event == Event::ArrowDown) {
		vterm_keyboard_key(vt_, VTERM_KEY_DOWN, mod);
		return true;
	}
	if (event == Event::ArrowLeft) {
		vterm_keyboard_key(vt_, VTERM_KEY_LEFT, mod);
		return true;
	}
	if (event == Event::ArrowRight) {
		vterm_keyboard_key(vt_, VTERM_KEY_RIGHT, mod);
		return true;
	}
	if (event == Event::Delete) {
		vterm_keyboard_key(vt_, VTERM_KEY_DEL, mod);
		return true;
	}
	if (event == Event::Home) {
		vterm_keyboard_key(vt_, VTERM_KEY_HOME, mod);
		return true;
	}
	if (event == Event::End) {
		vterm_keyboard_key(vt_, VTERM_KEY_END, mod);
		return true;
	}
	if (event == Event::PageUp) {
		vterm_keyboard_key(vt_, VTERM_KEY_PAGEUP, mod);
		return true;
	}
	if (event == Event::PageDown) {
		vterm_keyboard_key(vt_, VTERM_KEY_PAGEDOWN, mod);
		return true;
	}
	if (event == Event::Insert) {
		vterm_keyboard_key(vt_, VTERM_KEY_INS, mod);
		return true;
	}

	// Function keys F1-F12
	static const Event fkeys[] = {
		Event::F1, Event::F2, Event::F3, Event::F4,	 Event::F5,	 Event::F6,
		Event::F7, Event::F8, Event::F9, Event::F10, Event::F11, Event::F12,
	};
	for (int i = 0; i < 12; ++i) {
		if (event == fkeys[i]) {
			vterm_keyboard_key(vt_,
							   static_cast<VTermKey>(VTERM_KEY_FUNCTION(i + 1)),
							   mod);
			return true;
		}
	}

	// Character input
	if (event.is_character()) {
		std::string input = event.character();
		// Decode UTF-8 to codepoint for vterm
		const auto *bytes =
			reinterpret_cast<const unsigned char *>(input.data());
		uint32_t cp = 0;
		size_t len = input.size();

		if (len == 1) {
			cp = bytes[0];
		} else if (len == 2 && (bytes[0] & 0xE0) == 0xC0) {
			cp = ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
		} else if (len == 3 && (bytes[0] & 0xF0) == 0xE0) {
			cp = ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) |
				 (bytes[2] & 0x3F);
		} else if (len == 4 && (bytes[0] & 0xF8) == 0xF0) {
			cp = ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) |
				 ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
		}

		if (cp > 0) {
			vterm_keyboard_unichar(vt_, cp, mod);
			return true;
		}
	}

	// Pass any remaining input (Ctrl+key combos, Alt+key, etc.) directly
	// to the PTY as raw bytes. Skip null-only sequences (e.g. Event::Custom).
	if (!raw.empty() && raw.find_first_not_of('\0') != std::string::npos) {
		pty_.write(raw.data(), raw.size());
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------------
// Component — combines Renderer + CatchEvent
// ---------------------------------------------------------------------------

Component TerminalPanel::Component()
{
	// The bool overload of Renderer makes this component focusable,
	// so it receives keyboard events when selected in the tab container.
	auto impl = Renderer([this](bool focused) {
		auto elem = RenderScreen();
		if (focused)
			elem = elem | focus;
		return elem | reflect(box_);
	});
	return impl | CatchEvent([this](Event e) { return HandleEvent(e); });
}

bool TerminalPanel::WantsEvent(ftxui::Event event) const
{
	if (!spawned_.load() || ptyDead_.load())
		return false;

	// Always intercept Ctrl+T so HandleEvent can toggle capture mode.
	if (event.input() == std::string("\x14"))
		return true;

	// When not capturing, let the UI handle input (tab switching, etc.).
	if (!capturing_.load())
		return false;

	// Capture mode: claim all keyboard input for the terminal.
	// Skip null-only events (e.g. Event::Custom used for redraws).
	if (event.is_character())
		return true;
	const std::string &inp = event.input();
	if (!inp.empty() && inp.find_first_not_of('\0') != std::string::npos)
		return true;
	return false;
}
