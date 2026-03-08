/**
 * @file terminal_panel.cpp
 * @brief Terminal panel implementation — PTY + VTerm + FTXUI rendering.
 *
 * Wires together PtyHandler (platform PTY), libvterm (ANSI parser), and
 * FTXUI (TUI rendering) to provide an embedded terminal emulator inside
 * the Inferix application.
 */

#include "terminal_panel.h"
#include "pty_handler.h"

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

static void Vterm_output_cb(const char *s, size_t len, void * /*user*/)
{
	PtyHandler::instance().write(s, len);
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

TerminalPanel::TerminalPanel(ScreenInteractive &screen) : screen_(screen)
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

	vt_ = vterm_new(rows_, cols_);
	vterm_set_utf8(vt_, 1);

	vts_ = vterm_obtain_screen(vt_);
	vterm_screen_reset(vts_, 1);

	vterm_output_set_callback(vt_, Vterm_output_cb, nullptr);

	if (!PtyHandler::instance().spawn(cols_, rows_)) {
		vterm_free(vt_);
		vt_ = nullptr;
		vts_ = nullptr;
		return;
	}

	stop_flag_.store(false);
	pty_dead_.store(false);
	spawned_.store(true);

	read_thread_ = std::thread(&TerminalPanel::ReadLoop, this);
}

// ---------------------------------------------------------------------------
// Shutdown — stop thread, close PTY, free vterm
// ---------------------------------------------------------------------------

void TerminalPanel::Shutdown()
{
	if (!spawned_.load())
		return;

	stop_flag_.store(true);
	cv_.notify_one();

	if (read_thread_.joinable())
		read_thread_.join();

	PtyHandler::instance().close();

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

	while (!stop_flag_.load()) {
		int n = PtyHandler::instance().read(buf, sizeof(buf));

		if (n > 0) {
			std::lock_guard<std::mutex> lock(vterm_mutex_);
			vterm_input_write(vt_, buf, static_cast<size_t>(n));
			screen_.PostEvent(Event::Custom);
		} else if (n == 0) {
			// No data available — sleep briefly
			std::unique_lock<std::mutex> lock(cv_mutex_);
			cv_.wait_for(lock, std::chrono::milliseconds(16), [this] {
				return stop_flag_.load();
			});
		} else {
			// PTY error / closed
			if (!PtyHandler::instance().is_alive()) {
				pty_dead_.store(true);
				screen_.PostEvent(Event::Custom);
				break;
			}
		}
	}
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

	if (pty_dead_.load()) {
		return window(text(""),
					  center(text("Shell process has exited.") | dim),
					  EMPTY) |
			   flex;
	}

	std::lock_guard<std::mutex> lock(vterm_mutex_);

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

			cells.push_back(elem);

			// Advance by cell width (wide chars occupy 2 columns)
			col += (cell.width > 1) ? cell.width : 1;
		}

		lines.push_back(hbox(std::move(cells)));
	}

	return window(text(""), vbox(std::move(lines)), EMPTY) | flex;
}

// ---------------------------------------------------------------------------
// HandleEvent — keyboard input → vterm → PTY
// ---------------------------------------------------------------------------

bool TerminalPanel::HandleEvent(Event event)
{
	if (!spawned_.load() || pty_dead_.load())
		return false;

	std::lock_guard<std::mutex> lock(vterm_mutex_);

	VTermModifier mod = VTERM_MOD_NONE;

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
		return elem;
	});
	return impl | CatchEvent([this](Event e) { return HandleEvent(e); });
}

bool TerminalPanel::WantsEvent(ftxui::Event event) const
{
	if (!spawned_.load() || pty_dead_.load())
		return false;
	if (event.is_character())
		return true;
	if (event == Event::Return || event == Event::Backspace ||
		event == Event::Delete || event == Event::ArrowUp ||
		event == Event::ArrowDown || event == Event::ArrowLeft ||
		event == Event::ArrowRight || event == Event::Home ||
		event == Event::End || event == Event::PageUp ||
		event == Event::PageDown || event == Event::Insert ||
		event == Event::Escape || event == Event::Tab)
		return true;
	return false;
}
