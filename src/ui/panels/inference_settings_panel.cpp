/**
 * @file inference_settings_panel.cpp
 * @brief Inference settings panel implementation.
 *
 * Implements a panel that displays runtime inference parameters
 * such as temperature, top_p, max_tokens, and other model settings.
 */

#include "inference_settings_panel.h"

#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Element InferenceSettingsPanel::Render() {
	Elements settings;

	// Temperature setting (0.0 - 1.0)
	settings.push_back(hbox({
		text("Temperature") | bold,
		separatorEmpty(),
		text("0.7") | color(Color::CyanLight),
		separatorEmpty(),
		gaugeRight(0.7f) | color(Color::GreenLight),
	}));

	settings.push_back(separatorLight());

	// Top_p setting (0.0 - 1.0)
	settings.push_back(hbox({
		text("Top P") | bold,
		separatorEmpty(),
		text("0.9") | color(Color::MagentaLight),
		separatorEmpty(),
		gaugeRight(0.9f) | color(Color::GreenLight),
	}));

	settings.push_back(separatorLight());

	// Max tokens setting
	settings.push_back(hbox({
		text("Max Tokens") | bold,
		separatorEmpty(),
		text("2048") | color(Color::CyanLight),
	}));

	settings.push_back(separatorLight());

	// Repeat penalty setting
	settings.push_back(hbox({
		text("Repeat Penalty") | bold,
		separatorEmpty(),
		text("1.1") | color(Color::MagentaLight),
	}));

	settings.push_back(separatorLight());

	// Stop sequence setting
	settings.push_back(hbox({
		text("Stop Sequences") | bold,
		separatorEmpty(),
		text("") | color(Color::CyanLight),
	}));

	settings.push_back(separatorLight());

	// Logits processor
	settings.push_back(hbox({
		text("Logits Processor") | bold,
		separatorEmpty(),
		text("Enabled") | color(Color::GreenLight),
	}));

	return window(text("Inference Settings") | bold,
				  vbox({
					  separatorLight(),
					  flex(vbox(settings)),
					  separatorLight(),
				  }));
}
