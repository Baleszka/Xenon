#pragma once
#include <imgui-cocos.hpp>

void setupGUI();

inline void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

#define IM_COUNTOF(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))