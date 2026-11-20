#include "themes.hpp"

REGISTER_THEME("Xenon")
{
	auto& style = ImGui::GetStyle();
	ImGui::StyleColorsDark();

	style.SeparatorTextAlign = { 0.5f, 0.5f };
	style.WindowTitleAlign = { 0.5f, 0.5f };
	style.FramePadding = { 8.f, 4.f };
	style.WindowPadding = { 10.f, 10.f };

	style.WindowRounding = 20.f;
	style.ChildRounding = 16.f;
	style.FrameRounding = 10.f;
	style.PopupRounding = 16.f;
	style.GrabRounding = 10.f;
	style.TabRounding = 12.f;
	style.ScrollbarRounding = 12.f;
	style.WindowBorderSize = 1.f;
	style.ChildBorderSize = 0.f;
	style.PopupBorderSize = 1.f;
	style.FrameBorderSize = 1.f;
	style.TabBorderSize = 0.f;
	style.SeparatorTextBorderSize = 1.f;
	style.ItemSpacing = { 8.f, 6.f };
	style.ItemInnerSpacing = { 6.f, 5.f };
	style.ScrollbarSize = 12.f;
	style.GrabMinSize = 12.f;
	style.DisabledAlpha = 0.55f;

	const auto surface = themes::hexColor(0x0C0E13);
	const auto surfaceContainerLow = themes::hexColor(0x11141B);
	const auto surfaceContainer = themes::hexColor(0x161A23);
	const auto surfaceContainerHigh = themes::hexColor(0x1D2230);
	const auto surfaceContainerHighest = themes::hexColor(0x232630);
	const auto onSurface = themes::hexColor(0xE8E9F1);
	const auto onSurfaceVariant = themes::hexColor(0xB8BACA);
	const auto primary = themes::hexColor(0xC3CBFF);
	const auto primaryContainer = themes::hexColor(0x2C3A6B);
	const auto onPrimaryContainer = themes::hexColor(0xDDE1FF);
	const auto secondaryContainer = themes::hexColor(0x2B2F3D);
	const auto tertiary = themes::hexColor(0x9FC6FF);
	const auto outline = themes::hexColor(0x3A3E4E);
	const auto outlineVariant = themes::hexColor(0x2A2E3B);

	auto& colors = style.Colors;

	colors[ImGuiCol_WindowBg] = themes::withAlpha(surface, 0.97f);
	colors[ImGuiCol_ChildBg] = themes::withAlpha(surface, 0.0f);
	colors[ImGuiCol_PopupBg] = themes::withAlpha(surfaceContainerHigh, 0.99f);
	colors[ImGuiCol_MenuBarBg] = surfaceContainer;
	colors[ImGuiCol_TableHeaderBg] = surfaceContainer;
	colors[ImGuiCol_TableRowBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.f, 1.f, 1.f, 0.025f);
	colors[ImGuiCol_TableBorderStrong] = outline;
	colors[ImGuiCol_TableBorderLight] = outlineVariant;

	colors[ImGuiCol_TitleBg] = surfaceContainerLow;
	colors[ImGuiCol_TitleBgActive] = surfaceContainerHigh;
	colors[ImGuiCol_TitleBgCollapsed] = themes::withAlpha(surface, 0.75f);
	colors[ImGuiCol_Border] = outlineVariant;
	colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);

	colors[ImGuiCol_FrameBg] = surfaceContainer;
	colors[ImGuiCol_FrameBgHovered] = surfaceContainerHigh;
	colors[ImGuiCol_FrameBgActive] = surfaceContainerHighest;

	colors[ImGuiCol_Button] = surfaceContainerHigh;
	colors[ImGuiCol_ButtonHovered] = primaryContainer;
	colors[ImGuiCol_ButtonActive] = themes::hexColor(0x3E4C8A);

	colors[ImGuiCol_CheckMark] = primary;
	colors[ImGuiCol_SliderGrab] = primary;
	colors[ImGuiCol_SliderGrabActive] = themes::hexColor(0xE1E5FF);
	colors[ImGuiCol_Header] = themes::withAlpha(primary, 0.14f);
	colors[ImGuiCol_HeaderHovered] = themes::withAlpha(primary, 0.24f);
	colors[ImGuiCol_HeaderActive] = themes::withAlpha(primary, 0.32f);
	colors[ImGuiCol_TextSelectedBg] = themes::withAlpha(primary, 0.30f);
	colors[ImGuiCol_TextLink] = primary;
	colors[ImGuiCol_InputTextCursor] = primary;
	colors[ImGuiCol_DragDropTarget] = primary;
	colors[ImGuiCol_NavCursor] = primary;
	colors[ImGuiCol_UnsavedMarker] = tertiary;

	colors[ImGuiCol_Text] = onSurface;
	colors[ImGuiCol_TextDisabled] = onSurfaceVariant;

	colors[ImGuiCol_Separator] = outlineVariant;
	colors[ImGuiCol_SeparatorHovered] = themes::withAlpha(primary, 0.40f);
	colors[ImGuiCol_SeparatorActive] = themes::withAlpha(primary, 0.70f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
	colors[ImGuiCol_ScrollbarGrab] = outline;
	colors[ImGuiCol_ScrollbarGrabHovered] = themes::hexColor(0x4B5470);
	colors[ImGuiCol_ScrollbarGrabActive] = primary;

	colors[ImGuiCol_ResizeGrip] = themes::withAlpha(primary, 0.30f);
	colors[ImGuiCol_ResizeGripHovered] = themes::withAlpha(primary, 0.50f);
	colors[ImGuiCol_ResizeGripActive] = themes::withAlpha(primary, 0.70f);

	colors[ImGuiCol_Tab] = ImVec4(0.f, 0.f, 0.f, 0.f);
	colors[ImGuiCol_TabHovered] = themes::withAlpha(primary, 0.12f);
	colors[ImGuiCol_TabSelected] = secondaryContainer;
	colors[ImGuiCol_TabSelectedOverline] = primary;
	colors[ImGuiCol_TabDimmed] = ImVec4(0.f, 0.f, 0.f, 0.f);
	colors[ImGuiCol_TabDimmedSelected] = themes::withAlpha(surfaceContainerHighest, 0.70f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = themes::withAlpha(primary, 0.50f);

	colors[ImGuiCol_PlotLines] = primary;
	colors[ImGuiCol_PlotLinesHovered] = onPrimaryContainer;
	colors[ImGuiCol_PlotHistogram] = themes::withAlpha(primary, 0.80f);
	colors[ImGuiCol_PlotHistogramHovered] = themes::hexColor(0xE1E5FF);
	colors[ImGuiCol_TreeLines] = outline;

	colors[ImGuiCol_NavWindowingHighlight] = themes::withAlpha(onSurface, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.f, 0.f, 0.f, 0.60f);
	colors[ImGuiCol_ModalWindowDimBg] = themes::withAlpha(surface, 0.65f);

	/* Those who know :shush:
	ke — 12:00 AM
	yo @ch(?)gh
	ch(?)gh [CATE],  — 12:00 AM
	yo
	ke — 12:00 AM
	Sell me this tcbot
	those who know
	ch(?)gh [CATE],  — 12:01 AM
	I don't think I know you
	[CATE], 
	ke — 12:01 AM
	those who know
	*/
}
