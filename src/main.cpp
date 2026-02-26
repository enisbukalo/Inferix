#include <memory>

#include "httplib.h"
#include "json.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

using namespace ftxui;

int main(){
	auto screen = ScreenInteractive::Fullscreen();

	// State
	int left_size = 20;
	int right_size = 20;
	int top_size = 10;
	int bottom_size = 10;

	// Renderers
	auto RendererInfo = [](const std::string& name, int* size){
		return Renderer([name, size]{
				return text(name + ": " + std::to_string(*size)) | center;
				});
	};
	auto middle = Renderer([] {return text("Middle") | center;});
	auto left = RendererInfo("Left", &left_size);
	auto right = RendererInfo("Right", &right_size);
	auto top = RendererInfo("Top", &top_size);
	auto bottom = RendererInfo("Bottom", &bottom_size);

	auto container = middle;
	container = ResizableSplitLeft(left, container, &left_size);
	container = ResizableSplitRight(right, container, &right_size);
	container = ResizableSplitTop(top, container, &top_size);
	container = ResizableSplitBottom(bottom, container, &bottom_size);

	auto renderer =
	    Renderer(container, [&] { return container->Render() | border; });

	screen.Loop(renderer);

	return 0;
}
