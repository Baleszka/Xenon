#include "layout.hpp"
#include "themes/themes.hpp"

using namespace geode::prelude;

namespace layout
{
    void draw()
    {
        static float textSize = 18;
        static bool showDemo = false;
        static bool replayMenu = true;
        static bool assistMenu = false;
        static bool macroMenu = false;
        static bool calcMenu = false;
        static bool settingsMenu = false;

        ImGui::SetNextWindowSize(ImVec2(565.0f, 400.0f), ImGuiCond_Once);
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

        if (ImGui::Begin("Xenon Alpha", nullptr, windowFlags))
        {
            ImGui::PushFont(NULL, textSize);
            
            const char* labels[] = { "Replay", "Assist", "Macro editor", "Calculator", "Settings" };
            bool* menus[] = { &replayMenu, &assistMenu, &macroMenu, &calcMenu, &settingsMenu };
            constexpr int count  = 5;

            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float avail   = ImGui::GetContentRegionAvail().x;

            float textTotal = 0.0f;
            for (int i = 0; i < count; i++) { textTotal += ImGui::CalcTextSize(labels[i]).x; }

            float extra = (avail - spacing * (count - 1) - textTotal) / count;

            for (int i = 0; i < count; i++)
            {
                if (i > 0) ImGui::SameLine();

                float buttonW = ImGui::CalcTextSize(labels[i]).x + extra;

                if (ImGui::Button(labels[i], ImVec2(buttonW, 0))) { for (int j = 0; j < count; j++) { *menus[j] = (i == j); } }
            }

            if (replayMenu)
            {
                auto comboFlags = ImGuiComboFlags_NoPreview;

                ImGui::SeparatorText("Replay##header");

                ImGui::InputTextWithHint("", "Replay name...", replayName, IM_COUNTOF(replayName)); ImGui::SameLine();
                if (ImGui::BeginCombo("##replaySelect", "", comboFlags))
                {
                    for (auto &replay : getReplays())
                    {
                        bool isSelected = (replayName == replay);
                        if (ImGui::Selectable(replay.c_str(), isSelected))
                        {
                            strncpy(replayName, replay.c_str(), IM_COUNTOF(replayName) - 1);
                            replayName[IM_COUNTOF(replayName) - 1] = '\0';
                        }
                        if (isSelected) { ImGui::SetItemDefaultFocus(); }
                    }
                    ImGui::EndCombo();
                } ImGui::SameLine();
                if (ImGui::Button("Save")) { saveMacro(getReplaysDir(), replayName, tps, cbf, macro); } ImGui::SameLine();
                if (ImGui::Button("Load"))
                {
                    loadMacro(getReplaysDir(), replayName);
                    if (!patchesEnabled) { applyPatches(); }
                }

                if (ImGui::RadioButton("Record", &replayStatus, 0)) 
                { 
                    if (!patchesEnabled) { applyPatches(); }
                    // Clear inputs after current frame if set to record
                    if (auto pl = PlayLayer::get()) { while (!macro.empty() && macro.back().frame >= getFrame()) { macro.pop_back(); } }
                } ImGui::SameLine();
                if (ImGui::RadioButton("Play", &replayStatus, 1)) { if (!patchesEnabled) { applyPatches(); } } ImGui::SameLine();
                if (ImGui::RadioButton("Disable", &replayStatus, 2)) { if (patchesEnabled) { disablePatches(); } frameStepper = false; }

                ImGui::Separator();

                ImGui::DragInt("TPS", &tps, 1.0f, 1, 1000000); ImGui::SameLine();
                gui::HelpMarker("Bypasses the 240 tickrate limit.");

                ImGui::DragFloat("Speedhack", &speedhack, 0.01f, 0.01f, 10.0f, "%.2fx");

                ImGui::Checkbox("Speedhack audio", &speedhackAudio);

                ImGui::Separator();

                bool cbfCheck = cbf;
                if (ImGui::Checkbox("CBF", &cbfCheck))
                {
                    if (cbfCheck)
                    {
                        cbf = true;
                        cbfMacro.clear();
                        for (int i = 0; i < macro.size(); i++) { cbfMacro.push_back({float(macro[i].frame), macro[i].isHolding, macro[i].isPlayer1}); }
                    }
                    else if (!cbfMacro.empty()) { ImGui::OpenPopup("CBF"); }
                    else { cbf = false; }
                } ImGui::SameLine();
                gui::HelpMarker("Enables Xenon's own implementation of CBF.\nDisable the CBF mod by Syzzi and RobTop's CBS.\nChange CBF frame clicked on in Macro Editor.");

                ImGui::Checkbox("Frame stepper", &frameStepper); ImGui::SameLine();
                gui::HelpMarker("Enables frame stepper.\nDefault keybinds:\nC -> Advance\nV -> Toggle");

                ImGui::Checkbox("Block Inputs", &blockInputs); ImGui::SameLine();
                gui::HelpMarker("Blocks inputs while replaying a macro.");

                if (ImGui::BeginPopupModal("CBF", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
                {
                    ImGui::Text("Are you sure you want to disable CBF?\nSub-frame clicked on will be discarded!\nDisabling CBF may cause your macro to break.");
                    ImGui::Separator();

                    if (ImGui::Button("Yes"))
                    {
                        cbf = false;
                        macro.clear();
                        for (int i = 0; i < cbfMacro.size(); i++) { macro.push_back({uint64_t(cbfMacro[i].preciseFrame), cbfMacro[i].isHolding, cbfMacro[i].isPlayer1}); }
                        ImGui::CloseCurrentPopup();
                    } ImGui::SameLine();
                    if (ImGui::Button("No")) { ImGui::CloseCurrentPopup(); }

                    ImGui::EndPopup();
                }
            }

            if (assistMenu)
            {

                ImGui::SeparatorText("Assist##header");

                ImGui::BeginChild("AssistMenu", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

                ImGui::Checkbox("Show hitboxes", &showHitboxes); ImGui::SameLine();
                ImGui::Checkbox("Hitbox trail", &hitboxTrail); ImGui::SameLine();
                gui::HelpMarker("Draws a trail of the player's hitbox.");

                ImGui::DragFloat("Width", &hitboxWidth, 0.01f, 0.01f, 1.0f, "%.2f"); ImGui::SameLine();
                gui::HelpMarker("How wide the hitbox outline is drawn.");

                ImGui::DragInt("Trail length", &trailLength, 1, 0, 65536); ImGui::SameLine();
                gui::HelpMarker("How many instances of the trail can be drawn at a time.\nHigher trail length may affect performance.\nTrail length is counted for each individual player in dual mode.");

                ImGui::ColorEdit3("Hazard", hazardCol); ImGui::SameLine();
                gui::HelpMarker("Spikes, sawblades, player");

                ImGui::ColorEdit3("Hazard rotated", hazardRotCol); ImGui::SameLine();
                gui::HelpMarker("Rotated hazards, rotated player");

                ImGui::ColorEdit3("Solid", solidCol); ImGui::SameLine();
                gui::HelpMarker("Blocks, inner player");

                ImGui::ColorEdit3("Interactable", interactableCol); ImGui::SameLine();
                gui::HelpMarker("Orbs, pads, portals, letter blocks");

                ImGui::ColorEdit3("Passable", passableCol); ImGui::SameLine();
                gui::HelpMarker("Objects with passable setting");

                ImGui::SeparatorText("Player Hitboxes##header");
                ImGui::Checkbox("Normal", &showPlayer); ImGui::SameLine();
                ImGui::Checkbox("Rotated", &showPlayerRot); ImGui::SameLine();
                ImGui::Checkbox("Inner", &showPlayerInner); ImGui::SameLine();
                ImGui::Checkbox("Circle", &showPlayerCircle);

                ImGui::SeparatorText("Autoclicker##header");

                ImGui::Text("Player 1");

                if (ImGui::Checkbox("Enabled##P1", &player1)) { if (!player1) { counterP1 = 0; } } ImGui::SameLine();
                ImGui::Checkbox("Swift##Autoclickerp1", &p1Swift); ImGui::SameLine();
                gui::HelpMarker("Does a swift on the frames instead of alternating\nbetween clicking and releasing.");

                ImGui::Text("Hold for"); ImGui::SameLine();
                ImGui::DragInt("##Holdp1", &p1HoldFor, 1, 1, 100000, "%i Frames");

                ImGui::Text("Release for"); ImGui::SameLine();
                ImGui::DragInt("##ReleaseP1", &p1ReleaseFor, 1, 1, 100000, "%i Frames");

                ImGui::Spacing();

                ImGui::Text("Player 2");

                if (ImGui::Checkbox("Enabled##P2", &player2)) { if (!player2) { counterP2 = 0; } } ImGui::SameLine();
                ImGui::Checkbox("Swift##Autoclickerp2", &p2Swift); ImGui::SameLine();
                gui::HelpMarker("Does a swift on the frames instead of alternating\nbetween clicking and releasing.");

                ImGui::Text("Hold for"); ImGui::SameLine();
                ImGui::DragInt("##HoldP2", &p2HoldFor, 1, 1, 100000, "%i Frames");

                ImGui::Text("Release for"); ImGui::SameLine();
                ImGui::DragInt("##ReleaseP2", &p2ReleaseFor, 1, 1, 100000, "%i Frames");

                ImGui::EndChild();
            }

            if (macroMenu)
            {
                ImGui::SeparatorText("Macro editor##header");

                if (!cbf)
                {
                    if (macro.empty()) { ImGui::Text("No macro loaded."); }
                    else
                    {
                        ImGui::BeginChild("MacroEditor", ImVec2(ImGui::GetContentRegionAvail()));

                        for (int i = 0; i < macro.size(); i++)
                        {
                            ImGui::PushID(i);

                            ImGui::DragScalar("Frame", ImGuiDataType_U64, &macro[i].frame);
                            ImGui::Checkbox("isHolding", &macro[i].isHolding); ImGui::SameLine();
                            ImGui::Checkbox("isPlayer1", &macro[i].isPlayer1);

                            if (ImGui::Button("Remove")) { macro.erase(macro.begin() + i); } ImGui::SameLine();
                            if (ImGui::Button("Add Below")) { macro.insert(macro.begin() + i + 1, {macro[i].frame, !macro[i].isHolding, macro[i].isPlayer1}); }

                            ImGui::Separator();
                            ImGui::PopID();
                        }

                        ImGui::EndChild();
                    }
                }
                else
                {
                    if (cbfMacro.empty()) { ImGui::Text("No macro loaded."); }

                    else
                    {
                        ImGui::BeginChild("MacroEditor", ImVec2(ImGui::GetContentRegionAvail()));

                        for (int i = 0; i < cbfMacro.size(); i++)
                        {
                            ImGui::PushID(i);

                            ImGui::DragScalar("Frame", ImGuiDataType_Double, &cbfMacro[i].preciseFrame, 0.0001f, nullptr, nullptr, "%.4f");
                            ImGui::Checkbox("isHolding", &cbfMacro[i].isHolding); ImGui::SameLine();
                            ImGui::Checkbox("isPlayer1", &cbfMacro[i].isPlayer1);

                            if (ImGui::Button("Remove")) { cbfMacro.erase(cbfMacro.begin() + i); } ImGui::SameLine();
                            if (ImGui::Button("Add Below")) { cbfMacro.insert(cbfMacro.begin() + i + 1, {cbfMacro[i].preciseFrame, !cbfMacro[i].isHolding, cbfMacro[i].isPlayer1}); }

                            ImGui::Text("Put into calculator as"); ImGui::SameLine();
                            if (ImGui::Button("Earliest")) { firstFrame = cbfMacro[i].preciseFrame; } ImGui::SameLine();
                            if (ImGui::Button("Latest")) { lastFrame = cbfMacro[i].preciseFrame; }

                            ImGui::Separator();
                            ImGui::PopID();
                        }

                        ImGui::EndChild();
                    }
                }
            }

            if (calcMenu)
            {
                ImGui::SeparatorText("Calculator##header");

                ImGui::DragScalar("First Frame", ImGuiDataType_Double, &firstFrame, 0.0001f, nullptr, nullptr, "%.4f"); ImGui::SameLine();
                gui::HelpMarker("First frame available to click on.");

                ImGui::DragScalar("Last Frame", ImGuiDataType_Double, &lastFrame, 0.0001f, nullptr, nullptr, "%.4f"); ImGui::SameLine();
                gui::HelpMarker("Last frame available to click on.");

                ImGui::InputInt("TPS", &tpsCalc); ImGui::SameLine();
                gui::HelpMarker("Tickrate used to calculate.");

                float framesAvail = calcFramesAvailable(firstFrame, lastFrame);

                ImGui::SeparatorText("Result");

                ImGui::Text("Frames available: %.4f", framesAvail);
                ImGui::Text("Time available: %fms", calcTimeAvailable(framesAvail, tpsCalc) * 1000.0f);
                ImGui::Text("Frame perfect on: %fhz", calcFramePerfectOn(framesAvail, tpsCalc));
                ImGui::TextDisabled("(Accuracy)");

                if (ImGui::BeginItemTooltip())
                {
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    ImGui::TextUnformatted("Calculations may not be 100% accurate due to\nfloating-point inaccuracy.");
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
            }

            if (settingsMenu)
            {
                ImGui::SeparatorText("Settings##header");

                ImGui::BeginChild("SettingsMenu", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

                ImGui::Text("GUI");
                ImGui::Separator();

                themes::themePicker("Theme");
                
                ImGui::Spacing(); ImGui::Spacing();

                ImGui::Text("Labels");

                ImGui::Separator();
                
                ImGui::Checkbox("Enabled", &labelsEnabled);
                ImGui::Spacing();
                ImGui::Text("Player");

                ImGui::Checkbox("Position", &positionLabels.enabled);
                if (positionLabels.enabled)
                {
                    ImGui::RadioButton("Top left##position", &positionLabels.position, 0); ImGui::SameLine();
                    ImGui::RadioButton("Top right##position", &positionLabels.position, 1); ImGui::SameLine();
                    ImGui::RadioButton("Bottom left##position", &positionLabels.position, 2); ImGui::SameLine();
                    ImGui::RadioButton("Bottom right##position", &positionLabels.position, 3);
                    
                    ImGui::DragFloat("Opacity##position", &positionLabels.opacity, 0.01f, 0.0f, 1.0f, "%.2f");
                    ImGui::DragInt("Size##position", &positionLabels.size, 1, 0, 50);
                }

                ImGui::Checkbox("Velocity", &velocityLabel.enabled);
                if (velocityLabel.enabled)
                {
                    ImGui::RadioButton("Top left##velocity", &velocityLabel.position, 0); ImGui::SameLine();
                    ImGui::RadioButton("Top right##velocity", &velocityLabel.position, 1); ImGui::SameLine();
                    ImGui::RadioButton("Bottom left##velocity", &velocityLabel.position, 2); ImGui::SameLine();
                    ImGui::RadioButton("Bottom right##velocity", &velocityLabel.position, 3);
                    ImGui::DragFloat("Opacity##velocity##velocity", &velocityLabel.opacity, 0.01f, 0.0f, 1.0f, "%.2f");
                    ImGui::DragInt("Size##velocity", &velocityLabel.size, 1, 0, 50);
                }

                ImGui::Checkbox("Rotation", &rotationLabel.enabled);
                if (rotationLabel.enabled)
                {
                    ImGui::RadioButton("Top left##rotation", &rotationLabel.position, 0); ImGui::SameLine();
                    ImGui::RadioButton("Top right##rotation", &rotationLabel.position, 1); ImGui::SameLine();
                    ImGui::RadioButton("Bottom left##rotation", &rotationLabel.position, 2); ImGui::SameLine();
                    ImGui::RadioButton("Bottom right##rotation", &rotationLabel.position, 3);
                    ImGui::DragFloat("Opacity##rotation", &rotationLabel.opacity, 0.01f, 0.0f, 1.0f, "%.2f");
                    ImGui::DragInt("Size##rotation", &rotationLabel.size, 1, 0, 50);
                }

                ImGui::Spacing();

                ImGui::Text("Game");

                ImGui::Checkbox("Tick", &tickLabel.enabled);
                if (tickLabel.enabled)
                {
                    ImGui::RadioButton("Top left##tick", &tickLabel.position, 0); ImGui::SameLine();
                    ImGui::RadioButton("Top right##tick", &tickLabel.position, 1); ImGui::SameLine();
                    ImGui::RadioButton("Bottom left##tick", &tickLabel.position, 2); ImGui::SameLine();
                    ImGui::RadioButton("Bottom right##tick", &tickLabel.position, 3);
                    ImGui::DragFloat("Opacity##tick", &tickLabel.opacity, 0.01f, 0.0f, 1.0f, "%.2f");
                    ImGui::DragInt("Size##tick", &tickLabel.size, 1, 0, 50);
                }

                ImGui::Checkbox("Seed", &seedLabel.enabled);
                if (seedLabel.enabled)
                {
                    ImGui::RadioButton("Top left##seed", &seedLabel.position, 0); ImGui::SameLine();
                    ImGui::RadioButton("Top right##seed", &seedLabel.position, 1); ImGui::SameLine();
                    ImGui::RadioButton("Bottom left##seed", &seedLabel.position, 2); ImGui::SameLine();
                    ImGui::RadioButton("Bottom right##seed", &seedLabel.position, 3);
                    ImGui::DragFloat("Opacity##velocity##seed", &seedLabel.opacity, 0.01f, 0.0f, 1.0f, "%.2f");
                    ImGui::DragInt("Size##seed", &seedLabel.size, 1, 0, 50);
                }

                ImGui::Spacing(); ImGui::Spacing();

                ImGui::Text("Miscellaneous");
                ImGui::Separator();

                if (ImGui::Button("Open Xenon folder")) { geode::utils::file::openFolder(Mod::get()->getPersistentDir()); }

                if (ImGui::Button("Crash game")) { ImGui::OpenPopup("Crash game"); }

                if (ImGui::BeginPopupModal("Crash game", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
                {
                    ImGui::Text("Are you sure you want to crash the game?\nUnsaved progress WILL be lost.\nUseful for looking at stack trace.");

                    ImGui::Separator();

                    if (ImGui::Button("Yes")) { volatile int *p = nullptr; *p = 42; }
                    ImGui::SameLine();
                    if (ImGui::Button("No")) { ImGui::CloseCurrentPopup(); }

                    ImGui::EndPopup();
                }

                ImGui::EndChild();
            }

            ImGui::PopFont();
        }

        ImGui::End();
    }
}