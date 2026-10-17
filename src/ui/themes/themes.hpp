#pragma once

#include <imgui-cocos.hpp>
#include <string>
#include <vector>

namespace themes
{
	struct Theme
	{
		std::string name;
		void (*apply)();
	};

	std::vector<Theme>& getThemes();
	std::size_t getThemeCount();
	std::size_t& currentThemeIndex();

	inline void registerTheme(const char* name, void (*apply)())
	{
		getThemes().push_back({ name, apply });
	}

	void applyTheme(std::size_t index);
	bool applyThemeByName(const std::string& name);

	void applySavedThemeOnce();

	bool themePicker(const char* label = "Theme");
}

#define REGISTER_THEME(nameStr) \
	static void registeredThemeFn(); \
	namespace \
	{ \
		struct ThemeRegistrar \
		{ \
			ThemeRegistrar() \
			{ \
				themes::registerTheme(nameStr, &registeredThemeFn); \
			} \
		}; \
		static ThemeRegistrar themeRegistrarInstance; \
	} \
	static void registeredThemeFn()