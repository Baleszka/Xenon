#include "themes.hpp"

#include <imgui-cocos.hpp>

using namespace geode::prelude;

namespace themes
{
	std::vector<Theme>& getThemes()
	{
		static std::vector<Theme> themes;
		return themes;
	}

	std::size_t getThemeCount()
	{
		return getThemes().size();
	}

	std::size_t& currentThemeIndex()
	{
		static std::size_t idx = Mod::get()->getSavedValue<std::size_t>("xenon-theme", 0);
		return idx;
	}

	void applyTheme(std::size_t index)
	{
		auto& list = getThemes();
		if (index < list.size())
		{
			log::info("Applying theme {}", index);
			list[index].apply();
			currentThemeIndex() = index;
			Mod::get()->setSavedValue<int>("xenon-theme", index);
			static_cast<void>(Mod::get()->saveData());
		}
	}

	bool applyThemeByName(const std::string& name)
	{
		for (std::size_t i = 0; i < getThemes().size(); i++)
		{
			if (getThemes()[i].name == name)
			{
				applyTheme(i);
				return true;
			}
		}
		return false;
	}

	void applySavedThemeOnce()
	{
		static bool applied = false;
		if (applied)
		{
			return;
		}
		applied = true;

		int saved = Mod::get()->getSavedValue<int>("xenon-theme", 0);
		log::info("loading theme: {} (count: {})", saved, getThemeCount());
		if (saved < 0 || static_cast<std::size_t>(saved) >= getThemeCount())
		{
			saved = 0;
		}
		applyTheme(static_cast<std::size_t>(saved));
	}

	bool themePicker(const char* label)
	{
		auto& list = getThemes();
		const char* current = list.empty() ? "?" : list[themes::currentThemeIndex()].name.c_str();

		if (!ImGui::BeginCombo(label, current))
		{
			return false;
		}

		for (std::size_t i = 0; i < list.size(); i++)
		{
			bool selected = (i == currentThemeIndex());
			if (ImGui::Selectable(list[i].name.c_str(), selected))
			{
				// always re-apply, even picking the currently active theme
				applyTheme(i);
			}
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();

		Mod::get()->setSavedValue<int>("xenon-theme", currentThemeIndex());

		return true;
	}

	// builtin default imgui theme
	REGISTER_THEME("Default")
	{
		ImGui::StyleColorsDark();

		auto& s = ImGui::GetStyle();
		s.FrameRounding    = 0;
		s.WindowRounding   = 0;
		s.ChildRounding    = 0;
		s.PopupRounding    = 0;
		s.GrabRounding     = 0;
		s.TabRounding      = 0;
		s.ScrollbarRounding = 0;
		s.WindowBorderSize = 1;
		s.ChildBorderSize  = 1;
		s.PopupBorderSize  = 1;
		s.FramePadding     = { 7.25f, 3 };
		s.SeparatorTextAlign = { 0.5f, 0.5f };
		s.WindowTitleAlign   = { 0.5f, 0.5f };
	}
}