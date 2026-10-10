#include "replay.hpp"
#include "frameHook.hpp"
#include "../ui/gui.hpp"
#include "tpsBypass.hpp"
#include "cbf.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool ignoreInputs = false;

class $modify(RPGJBaseGameLayer, GJBaseGameLayer)
{
    void handleButton(bool down, int button, bool isPlayer1)
    {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
        if (replayStatus == 0 && button == static_cast<int>(PlayerButton::Jump) && !ignoreInputs) { addInput(g_frame, down, isPlayer1); } // record
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick)
    {
        stepInputs.clear();
        auto pl = PlayLayer::get();
        if (replayStatus == 1 && pl && static_cast<GJBaseGameLayer *>(pl) == this)
        {
            if (cbf)
            {
                for (auto &in : cbfMacro)
                {
                    auto step = static_cast<unsigned long long>(in.preciseFrame);
                    if (step != g_frame) { continue; }
                    double alpha = in.preciseFrame - step;
                    if (alpha < 1e-4) { GJBaseGameLayer::handleButton(in.isHolding, static_cast<int>(PlayerButton::Jump), in.isPlayer1); }
                    else { stepInputs.push_back({alpha, in.isHolding, in.isPlayer1}); }
                }
            }
            else
            {
                for (auto &in : macro)
                {
                    if (in.frame == g_frame) { GJBaseGameLayer::handleButton(in.isHolding, static_cast<int>(PlayerButton::Jump), in.isPlayer1); }
                }
            }
        }
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(ReplayPlayLayer, PlayLayer)
{
    void resetLevel()
    {
        // Whoever is about to read the following lines of code, I am so sorry </3
        // Theres a million other better ways of doing everything i am doing here
        if (m_playerDied && replayStatus == 0)
        {
            // If you release after dying, the other parts of my code clear queuedButtons and you keep jumping after respawning
            // This fixes that :3
            // (i think)
            ignoreInputs = true;
            if (!m_queuedButtons.back().m_isPush && !m_queuedButtons.back().m_isPlayer2)
            {
                GJBaseGameLayer::handleButton(false, static_cast<int>(PlayerButton::Jump), true);
                ignoreInputs = false;
            }
            else if (!m_queuedButtons.back().m_isPush && m_queuedButtons.back().m_isPlayer2)
            {
                GJBaseGameLayer::handleButton(false, static_cast<int>(PlayerButton::Jump), false);
                ignoreInputs = false;
            }
            else { ignoreInputs = false; } // if you let go while dead but also start holding before you respawn, it just won't record anything. Ts fixes it lel
        }

        PlayLayer::resetLevel();

        if (replayStatus == 0)
        {
            // remove inputs if the frame they were made on is bigger than the frame reset to
            if (!cbf)
            {
                while (!macro.empty() && macro.back().frame >= g_frame)
                {
                    macro.pop_back();
                }
            }
            else
            {
                while (!cbfMacro.empty() && cbfMacro.back().preciseFrame >= g_frame) { cbfMacro.pop_back(); }
            }

            // if last input is a release and queuedButtons' last element is also a release, clear queuedButtons so it doesn't add an unnecessary release
            if (!cbf)
            {
                if (!macro.empty() && !macro.back().isHolding && !m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
            }
            else
            {
                if (!cbfMacro.empty() && !cbfMacro.back().isHolding && !m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
            }

            // if release input was made after frame that you reset to and queueButtons' last input is a release, let queueButtons do the release
            // added for debugging purposes
            // else if (!macro.empty() && macro.back().isHolding && !m_queuedButtons.back().m_isPush) { log::info("Adding release from queuedButtons"); }

            // if youre a game mode whos release matters, if youre holding while you reset, it doesnt start holding again, rather it ignores it and only adds the release when you release
            // for gamemodes like wave, ship and robot
            // (Technically UFO release matters too because if you do a swift while UFO nothing happens)
            if (!cbf)
            {
                if (!macro.empty() && macro.back().isHolding && m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
            }
            else
            {
                if (!cbfMacro.empty() && cbfMacro.back().isHolding && m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
            }

            // if you reset to a frame that was before any inputs were made and queuedButtons has a release input, ignore it
            // fixes ghost release
            if (!cbf)
            {
                if (macro.empty() && !m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush)  { m_queuedButtons.clear(); }
            }
            else
            {
                if (cbfMacro.empty() && !m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
            }
        }
    }
};

void addInput(unsigned long long frame, bool isHolding, bool isPlayer1)
{
    if (!cbf) { macro.push_back({frame, isHolding, isPlayer1}); }
    else { cbfMacro.push_back({float(frame), isHolding, isPlayer1}); }
    // log::info("Added input on frame {} isHolding {} isPlayer1 {}", frame, isHolding, isPlayer1);
}

void simulateInput(bool isHolding, bool isPlayer1)
{
    auto playLayer = PlayLayer::get();
    if (!playLayer)
        return;
    playLayer->handleButton(isHolding, static_cast<int>(PlayerButton::Jump), isPlayer1); // simulates input :O
}
