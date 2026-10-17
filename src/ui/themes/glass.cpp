#include "themes.hpp"

REGISTER_THEME("Glass")
{
	auto& gui = ImGui::GetStyle();

	/*
		for other people making their themes preferably you
		should keep these 4 like here but if you want
		you can do whatever
	*/

	gui.SeparatorTextAlign = { 0.5f, 0.5f };
	gui.WindowTitleAlign = { 0.5f, 0.5f };
	gui.FramePadding = { 7.25f, 3 };
	gui.WindowPadding = { 6.f, 12.f };

	gui.WindowRounding = 18;
	gui.ChildRounding = 14;
	gui.FrameRounding = 10;
	gui.PopupRounding = 14;
	gui.GrabRounding = 10;
	gui.TabRounding = 12;
	gui.ScrollbarRounding = 10;
	gui.WindowBorderSize = 1;
	gui.ChildBorderSize = 1;
	gui.PopupBorderSize = 1;
	gui.ItemSpacing = { 9.f, 7.f };
	gui.ItemInnerSpacing = { 8.f, 5.f };
	gui.ScrollbarSize = 13;

	const auto windowBg = ImVec4(0.05f, 0.05f, 0.05f, 0.85f);
	const auto surface = ImVec4(0.1f, 0.1f, 0.1f, 0.75f);
	const auto accent = ImVec4(0.5f, 0.5f, 0.9f, 0.85f);
	const auto accentHi = ImVec4(0.5f, 0.9f, 1.f, 0.9f);

	auto& colors = gui.Colors;

	colors[ImGuiCol_WindowBg] = windowBg;
	colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.11f, 0.15f, 0.94f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.09f, 0.12f, 0.85f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.16f, 0.22f, 0.88f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.09f, 0.12f, 0.60f);

	colors[ImGuiCol_Border] = ImVec4(0.65f, 0.72f, 0.85f, 0.28f);

	colors[ImGuiCol_FrameBg] = surface;
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.18f, 0.24f, 0.80f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.24f, 0.32f, 0.85f);
	colors[ImGuiCol_Button] = surface;
	colors[ImGuiCol_ButtonHovered] = accent;
	colors[ImGuiCol_ButtonActive] = accentHi;
	colors[ImGuiCol_CheckMark] = accentHi;
	colors[ImGuiCol_SliderGrab] = accentHi;
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.62f, 0.76f, 1.00f, 0.95f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.f, 0.f, 0.f, 0.20f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.35f, 0.45f, 0.55f);
	colors[ImGuiCol_ScrollbarGrabHovered] = accent;
	colors[ImGuiCol_ScrollbarGrabActive] = accentHi;

	colors[ImGuiCol_Separator] = ImVec4(0.65f, 0.72f, 0.85f, 0.18f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.65f, 0.72f, 0.85f, 0.35f);
	colors[ImGuiCol_SeparatorActive] = accent;
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.26f, 0.42f, 0.50f);
	colors[ImGuiCol_HeaderHovered] = accent;
	colors[ImGuiCol_HeaderActive] = accentHi;

	colors[ImGuiCol_Text] = ImVec4(0.94f, 0.95f, 0.99f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.58f, 0.62f, 0.72f, 0.85f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.36f, 0.52f, 0.90f, 0.38f);

	colors[ImGuiCol_TableHeaderBg] = surface;
	colors[ImGuiCol_TableRowBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.f, 1.f, 1.f, 0.03f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.02f, 0.05f, 0.45f);
}