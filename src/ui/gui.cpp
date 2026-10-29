#include "gui.hpp"
#include "layout.hpp"
#include "themes/themes.hpp"

#include <asp/fs/std.hpp>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace geode::prelude;

namespace gui
{
	void setupDefaultStyle()
	{
		ImGui::StyleColorsDark();

		ImGui::GetStyle().FrameRounding = 0;
		ImGui::GetStyle().WindowRounding = 0;
		ImGui::GetStyle().PopupRounding = 0;
		ImGui::GetStyle().SeparatorTextAlign = { 0.5f, 0.5f };
		ImGui::GetStyle().WindowTitleAlign = { 0.5f, 0.5f };
		ImGui::GetStyle().FramePadding = { 7.25f, 3 };

		auto& io = ImGui::GetIO();
		io.MouseDrawCursor = true;

		auto fontPath = Mod::get()->getResourcesDir() / "DroidSans.ttf";
		if (std::filesystem::exists(fontPath.toStdPath().unwrap()))
		{
			ImFontConfig cfg;
			cfg.GlyphExtraAdvanceX = 2.0f;
			ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 18.0f, &cfg);
			if (font) { io.FontDefault = font; }
			else { io.Fonts->AddFontDefault(); }
		}
		else { io.Fonts->AddFontDefault(); }
	}

	void setupGui()
	{
		ImGuiCocos::get().setup([]
		{
			gui::setupDefaultStyle();
		}).draw([]
		{
			themes::applySavedThemeOnce();
			layout::draw();
		});

		ImGuiCocos::get().toggle(); // hide menu on startup
	}

	void toggleGui() { ImGuiCocos::get().toggle(); }
}

$on_game(Loaded) 
{
    listenForKeybindSettingPresses("open-menu", [](Keybind const& keybind, bool down, bool repeat, double timestamp) 
	{
        if (down && !repeat) { gui::toggleGui(); }
    });
}