/**
 * @file models_panel.cpp
 * @brief Models panel implementation.
 *
 * Implements a panel that displays a list of available AI/ML models
 * for inference with placeholder content.
 */

#include "models_panel.h"

#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Element ModelsPanel::Render()
{
	Elements model_entries;

	// Placeholder model entries - structure ready for future data
	model_entries.push_back(hbox({
		text("[ ]") | color(Color::Black),
		separatorEmpty(),
		text("model-name-v1.gguf") | color(Color::White),
	}));

	model_entries.push_back(hbox({
		text("[ ]") | color(Color::Black),
		separatorEmpty(),
		text("llama-2-7b-chat.gguf") | color(Color::White),
	}));

	model_entries.push_back(hbox({
		text("[ ]") | color(Color::Black),
		separatorEmpty(),
		text("mistral-7b-instruct.gguf") | color(Color::White),
	}));

	return window(text("Models") | bold,
				  vbox({
					  flex(vbox(model_entries)),
				  }));
}
