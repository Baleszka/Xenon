#include "gui.hpp"
#include "../info/playerInfo.hpp"
#include "../bot/frameHook.hpp"
#include "../calculator/calculator.hpp"
#include "../bot/replay.hpp"
#include "../bot/cbf.hpp"
#include "../assist/hitboxes.hpp"
#include "../bot/frameStepper.hpp"
#include "../bot/speedhack.hpp"
#include "../bot/xe.hpp"
#include "../bot/tpsBypass.hpp"
#include "../patches/releaseFix.hpp"
// #include "../assist/trajectory.hpp"
#include "../assist/autoclicker.hpp"

#include <imgui-cocos.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace geode::prelude;

void setupGUI()
{
    ImGuiCocos::get().setup([]
    {
        ImGui::StyleColorsDark();

        ImGui::GetStyle().FrameRounding = 4;
        ImGui::GetStyle().WindowRounding = 6;
        ImGui::GetStyle().PopupRounding = 6;
        ImGui::GetStyle().SeparatorTextAlign = {0.5f, 0.5f};
        ImGui::GetStyle().WindowTitleAlign = {0.5f, 0.5f};
        ImGui::GetStyle().FramePadding = {7.25f, 3};

        auto &io = ImGui::GetIO();
        io.MouseDrawCursor = true;
        auto fontPath = Mod::get()->getResourcesDir() / "DroidSans.ttf";
        if (std::filesystem::exists(fontPath))
        {
            ImFontConfig cfg;
            cfg.GlyphExtraAdvanceX = 2.0f; // letter spacing
            ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 18.0f, &cfg);
            if (font) { io.FontDefault = font; }
        }
        else { io.Fonts->AddFontDefault(); } 
    }).draw([]
    {

        static float textSize = 18;

        static bool showDemo = false;
        static bool replayMenu = true;
        static bool assistMenu = false;
        static bool infoMenu = false;
        static bool macroMenu = false;
        static bool calcMenu = false;
        static bool settingsMenu = false;
        
        ImGui::SetNextWindowSize(ImVec2(565.0f, 400.0f), ImGuiCond_Once);
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

        if (assistMenu) { windowFlags |= ImGuiWindowFlags_NoScrollbar; }

        if (ImGui::Begin("Xenon Alpha", nullptr, windowFlags))
        {
            ImGui::PushFont(NULL, textSize);
            if (ImGui::Button("Replay"))
            {
                replayMenu = true; 
                assistMenu = false;
                infoMenu = false;
                macroMenu = false;
                calcMenu = false;
                settingsMenu = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Assist"))
            {
                replayMenu = false;
                assistMenu = true;
                infoMenu = false;
                macroMenu = false;
                calcMenu = false;
                settingsMenu = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Info"))
            {
                replayMenu = false;
                assistMenu = false;
                infoMenu = true;
                macroMenu = false;
                calcMenu = false;
                settingsMenu = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Macro editor"))
            {
                replayMenu = false;
                assistMenu = false;
                infoMenu = false;
                macroMenu = true;
                calcMenu = false;
                settingsMenu = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Calculator"))
            {
                replayMenu = false;
                assistMenu = false;
                infoMenu = false;
                macroMenu = false;
                calcMenu = true;
                settingsMenu = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Settings"))
            {
                replayMenu = false;
                assistMenu = false;
                infoMenu = false;
                macroMenu = false;
                calcMenu = false;
                settingsMenu = true;
            }

            if (replayMenu)
            {
                auto comboFlags = ImGuiComboFlags_NoPreview;

                ImGui::SeparatorText("Replay##header");

                ImGui::InputTextWithHint("", "Replay name...", replayName, IM_COUNTOF(replayName)); ImGui::SameLine();

                if (ImGui::BeginCombo("##replaySelect", "", comboFlags))
                {
                    for (auto& replay : getReplays())
                    {
                        bool isSelected = (replayName == replay);
                        if (ImGui::Selectable(replay.c_str(), isSelected))
                        {
                            strncpy(replayName, replay.c_str(), IM_COUNTOF(replayName) - 1);
                            replayName[IM_COUNTOF(replayName) - 1] = '\0';
                        }
                        if (isSelected)
                        ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();
                if (ImGui::Button("Save")) { saveMacro(getReplaysDir(), replayName, tps, cbf, macro); } ImGui::SameLine();
                if (ImGui::Button("Load")) { loadMacro(getReplaysDir(), replayName); if (!patchesEnabled) { applyPatches(); } }

                if (ImGui::RadioButton("Record", &replayStatus, 0)) { if (!patchesEnabled) { applyPatches(); } } ImGui::SameLine();
                if (ImGui::RadioButton("Play", &replayStatus, 1)) { if (!patchesEnabled) { applyPatches(); } } ImGui::SameLine();
                if (ImGui::RadioButton("Disable", &replayStatus, 2)) { if (patchesEnabled) { disablePatches(); } frameStepper = false; }

                ImGui::Separator();

                ImGui::DragInt("TPS", &tps, 1.0f, 1, 1000000); ImGui::SameLine();
                HelpMarker("Bypasses the 240 tickrate limit.");

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
                }
                ImGui::SameLine(); 
                HelpMarker("Enables Xenon's own implementation of CBF.\nDisable the CBF mod by Syzzi and RobTop's CBS.\nChange CBF frame clicked on in Macro Editor.");

                ImGui::Checkbox("Frame stepper", &frameStepper); ImGui::SameLine();
                HelpMarker("Enables frame stepper.\nKeybinds:\nC -> Advance\nV -> Toggle\nB -> Step back");
                
                ImGui::Checkbox("Block Input", &blockInputs); ImGui::SameLine();
                HelpMarker("Blocks input when replaying a macro");

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
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("No")) { ImGui::CloseCurrentPopup(); }

                    ImGui::EndPopup();
                }

            }

            if (assistMenu)
            {
                
                ImGui::SeparatorText("Assist##header");

                ImGui::BeginChild("AssistMenu");

                ImGui::Checkbox("Show hitboxes", &showHitboxes); ImGui::SameLine();
                ImGui::Checkbox("Hitbox trail", &hitboxTrail); ImGui::SameLine();
                HelpMarker("Draws a trail of the player's hitbox.");

                ImGui::DragFloat("Width", &hitboxWidth, 0.01f, 0.01f, 1.0f, "%.2f"); ImGui::SameLine();
                HelpMarker("How wide the hitbox outline is drawn.");

                ImGui::DragInt("Trail length", &trailLength, 1, 0, 65536); ImGui::SameLine();
                HelpMarker("How many instances of the trail can be drawn at a time.\nHigher trail length may affect performance.\nTrail length is counted for each individual player in dual mode.");

                ImGui::ColorEdit3("Hazard", hazardCol); ImGui::SameLine();
                HelpMarker("Spikes, sawblades, player");

                ImGui::ColorEdit3("Hazard rotated", hazardRotCol); ImGui::SameLine();
                HelpMarker("Rotated hazards, rotated player");

                ImGui::ColorEdit3("Solid", solidCol); ImGui::SameLine();
                HelpMarker("Blocks, inner player");

                ImGui::ColorEdit3("Interactable", interactableCol); ImGui::SameLine();
                HelpMarker("Orbs, pads, portals, letter blocks");

                ImGui::ColorEdit3("Passable", passableCol); ImGui::SameLine();
                HelpMarker("Objects with passable setting");

                ImGui::SeparatorText("Player Hitboxes##header");
                ImGui::Checkbox("Normal", &showPlayer); ImGui::SameLine();
                ImGui::Checkbox("Rotated", &showPlayerRot); ImGui::SameLine();
                ImGui::Checkbox("Inner", &showPlayerInner); ImGui::SameLine();
                ImGui::Checkbox("Circle", &showPlayerCircle);

                /*
                ImGui::SeparatorText("Trajectory");

                ImGui::Checkbox("Show trajectory", &trajectory); ImGui::SameLine();
                HelpMarker("Simulates where the player would go\nif you were to click or release.");

                ImGui::Checkbox("Hold", &holdTrajectory); ImGui::SameLine();
                HelpMarker("Player's trajectory if holding."); ImGui::SameLine();
                ImGui::Checkbox("Relase", &releaseTrajectory); ImGui::SameLine();
                HelpMarker("Player's trajectory if released."); ImGui::SameLine();
                ImGui::Checkbox("Swift##trajectory", &swiftTrajectory); ImGui::SameLine();
                HelpMarker("Player's trajectory if click and release\nwas made in the same frame.");

                ImGui::DragFloat("Trail width", &trajectoryTrailWidth, 0.01f, 0.01f, 1.0f, "%.2f"); ImGui::SameLine();
                HelpMarker("How wide the trajectory trail is drawn.");

                ImGui::DragInt("Trajectory length", &trajectoryLength, 20, 0, 1000000); ImGui::SameLine();
                HelpMarker("How many frames forward to simulate\nthe trajectory for.\nHigher trail legth may affect performance.");
                */

                ImGui::SeparatorText("Autoclicker##header");
                
                ImGui::Text("Player 1");

                if (ImGui::Checkbox("Enabled##P1", &player1))
                {
                    if (!player1) { counterP1 = 0; }
                }
                ImGui::SameLine();         
                ImGui::Checkbox("Swift##Autoclickerp1", &p1Swift); ImGui::SameLine();
                HelpMarker("Does a swift on the frames instead of alternating\nbetween clicking and releasing.");

                ImGui::Text("Hold for"); ImGui::SameLine();
                ImGui::DragInt("##Holdp1", &p1HoldFor, 1, 1, 100000, "%i Frames");

                ImGui::Text("Release for"); ImGui::SameLine();
                ImGui::DragInt("##ReleaseP1", &p1ReleaseFor, 1, 1, 100000, "%i Frames");

                ImGui::Spacing();

                ImGui::Text("Player 2");

                if (ImGui::Checkbox("Enabled##P2", &player2))
                {
                    if (!player2) { counterP2 = 0; }
                }
                ImGui::SameLine();         
                ImGui::Checkbox("Swift##Autoclickerp2", &p2Swift); ImGui::SameLine();
                HelpMarker("Does a swift on the frames instead of alternating\nbetween clicking and releasing.");

                ImGui::Text("Hold for"); ImGui::SameLine();
                ImGui::DragInt("##HoldP2", &p2HoldFor, 1, 1, 100000, "%i Frames");

                ImGui::Text("Release for"); ImGui::SameLine();
                ImGui::DragInt("##ReleaseP2", &p2ReleaseFor, 1, 1, 100000, "%i Frames");
                
                ImGui::EndChild();
            }

            if (infoMenu)
            {
                if (auto playLayer = PlayLayer::get()) 
                {
                    if (auto player = playLayer->m_player1)
                    {
                        ImGui::SeparatorText("Position");

                        ImGui::Text("Position: P1(%f, %f)", getPlayerPos().x1, getPlayerPos().y1);
                        ImGui::Text("              P2(%f, %f)", getPlayerPos().x2, getPlayerPos().y2);

                        ImGui::SeparatorText("Velocity");
                        ImGui::Text("Velocity: P1(%f)", getPlayerVel().x);
                        ImGui::Text("              P2(%f)", getPlayerVel().y);

                        ImGui::SeparatorText("Rotation");
                        ImGui::Text("Rotation: P1(%f°)", getPlayerRot().x);
                        ImGui::Text("              P2(%f°)", getPlayerRot().y);

                        ImGui::SeparatorText("Other");
                        ImGui::Text("Tick: %i", getFrame());
                        ImGui::Text("Seed: %i", getRandomSeed());
                    }
                }
                else { ImGui::Text("Enter level to see info."); }
            }
            if (macroMenu)
            {
                ImGui::SeparatorText("Macro editor##header");

                if (!cbf)
                {
                    if (macro.empty()) { ImGui::Text("No replay loaded."); }
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
                    if (cbfMacro.empty()) { ImGui::Text("No replay loaded."); }

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
                HelpMarker("First frame available to click on.");

                ImGui::DragScalar("Last Frame", ImGuiDataType_Double, &lastFrame, 0.0001f, nullptr, nullptr, "%.4f"); ImGui::SameLine();
                HelpMarker("Last frame available to click on.");

                ImGui::InputInt("TPS", &tpsCalc); ImGui::SameLine();
                HelpMarker("Tickrate used to calculate.");

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

                if (ImGui::Button("Open Xenon folder")) { geode::utils::file::openFolder(Mod::get()->getPersistentDir()); }
                ImGui::ShowStyleSelector("Style");

                ImGui::Checkbox("Show Demo", &showDemo); ImGui::SameLine();
                HelpMarker("Shows ImGui Demo window.");

                if (ImGui::Button("Crash game"))
                {
                    ImGui::OpenPopup("Crash game");
                } ImGui::SameLine();
                HelpMarker("Crashes your game.");

                if (ImGui::BeginPopupModal("Crash game", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
                {
                    ImGui::Text("Are you sure you want to crash the game?\nUnsaved progress WILL be lost.\nUseful for looking at stack trace.");
                    ImGui::Separator();
                    if (ImGui::Button("Yes")) { volatile int* p = nullptr; *p = 42; }
                    ImGui::SameLine();
                    if (ImGui::Button("No")) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }

                ImGui::TextLinkOpenURL("My Ko-Fi", "https://ko-fi.com/baleszkadev");
                ImGui::TextLinkOpenURL("Discord server", "https://discord.gg/rDhuzHp3Xe");

            }

            ImGui::PopFont();
        }

        ImGui::End();

        if (showDemo) 
        {
            ImGui::ShowDemoWindow();
        } });
        
    ImGuiCocos::get().toggle(); // hide menu on startup
}

class $modify(ImGuiKeybindHook, cocos2d::CCKeyboardDispatcher)
{
    bool dispatchKeyboardMSG(cocos2d::enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double ts)
    {
        if (key == cocos2d::enumKeyCodes::KEY_F && isKeyDown)
        {
            ImGuiCocos::get().toggle(); // toggle menu
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, ts);
    }
};