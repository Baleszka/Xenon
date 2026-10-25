#pragma once

#include <imgui-cocos.hpp>

#include "../info/gameInfo.hpp"
#include "../info/labels.hpp"
#include "../calculator/calculator.hpp"
#include "../bot/replay.hpp"
#include "../bot/cbf.hpp"
#include "../assist/hitboxes.hpp"
#include "../bot/frameStepper.hpp"
#include "../bot/speedhack.hpp"
#include "../bot/xe.hpp"
#include "../bot/tpsBypass.hpp"
#include "../patches/releaseFix.hpp"
#include "../assist/autoclicker.hpp"

#include <Geode/modify/CCKeyboardDispatcher.hpp>

namespace gui
{
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
}

namespace layout
{
    void draw();
}