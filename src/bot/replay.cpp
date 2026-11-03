#include "replay.hpp"
#include "../info/gameInfo.hpp"
#include "../ui/gui.hpp"
#include "tpsBypass.hpp"
#include "cbf.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

bool ignoreInputs = false;

class $modify(RPGJBaseGameLayer, GJBaseGameLayer)
{
    void processQueuedButtons(float dt, bool clearInputQueue) 
    {
        if (replayStatus == 2) { return GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue); }

        if (blockInputs && replayStatus == 1) { m_queuedButtons.clear(); } // Block inputs during replay

        if (replayStatus == 0 && !ignoreInputs && !m_queuedButtons.empty())
        {
            for (auto &in : m_queuedButtons)
            {
                addInput(getFrame() - 1, in.m_isPush, !in.m_isPlayer2); // record
            }
        }
        
        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);
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
                    if (step != getFrame()) { continue; }
                    
                    double alpha = in.preciseFrame - step;

                    if (alpha < 1e-4) { GJBaseGameLayer::handleButton(in.isHolding, static_cast<int>(PlayerButton::Jump), in.isPlayer1); }
                    else { stepInputs.push_back({alpha, in.isHolding, in.isPlayer1}); }
                }
            }
            
            else
            {
                for (auto &in : macro)
                {
                    if (in.frame == getFrame()) { GJBaseGameLayer::handleButton(in.isHolding, static_cast<int>(PlayerButton::Jump), in.isPlayer1); }
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
        if (replayStatus == 0 || replayStatus == 1 && m_playerDied)
        {
            // Fixes the issue where if you release after dying, because of the other parts of my code clearing queuedButtons, you keep jumping after respawning
            ignoreInputs = true;
            if (!m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush && !m_queuedButtons.back().m_isPlayer2)
            {
                handleButton(false, static_cast<int>(PlayerButton::Jump), true);
                ignoreInputs = false;
            }
            else if (!m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush && m_queuedButtons.back().m_isPlayer2)
            {
                handleButton(false, static_cast<int>(PlayerButton::Jump), false);
                ignoreInputs = false;
            }
            // if you let go while dead but also start holding before you respawn, it just won't record anything. This fixes that
            else { ignoreInputs = false; }
        }

        PlayLayer::resetLevel();

        if (replayStatus == 0)
        {
            if (!cbf)
            {   
                // remove inputs if the frame they were made on is bigger than the frame reset to
                while (!macro.empty() && macro.back().frame >= getFrame()) { macro.pop_back(); }

                // Note: these are the last inputs saved in the macro after cleaned, not the actual last input made my the player
                input* lastP1Input = nullptr; input* lastP2Input = nullptr;

                for (size_t i = macro.size(); i-- > 0; )
                {
                    if (!lastP1Input && macro[i].isPlayer1) { lastP1Input = &macro[i]; }
                    if (!lastP2Input && !macro[i].isPlayer1) { lastP2Input = &macro[i]; }
                    if (lastP1Input && lastP2Input) break;
                }
                        
                // Check what the last inputs were for both players in the macro after clear
                // No need to restrict p2 hold to 2p dual mode beacuse they get logged seperately regardless
                bool p1Holding = lastP1Input && lastP1Input->isHolding;
                bool p2Holding = lastP2Input && lastP2Input->isHolding;

                // Delete queued inputs that match the input the player is already doing
                std::erase_if(m_queuedButtons, [&](const auto& b) { return b.m_isPush == (b.m_isPlayer2 ? p2Holding : p1Holding); });

                // If you reset to a frame that was before any inputs were made and queuedButtons has a release input, get rid of it
                // This fixes ghost releases
                if (macro.empty() && !m_queuedButtons.empty()) { std::erase_if(m_queuedButtons, [](const auto& b) { return !b.m_isPush; }); }
            }
            else
            {
                while (!cbfMacro.empty() && cbfMacro.back().preciseFrame >= getFrame()) { cbfMacro.pop_back(); }
                cbfInput* lastP1Input = nullptr; cbfInput* lastP2Input = nullptr;

                for (size_t i = cbfMacro.size(); i-- > 0; )
                {
                    if (!lastP1Input && cbfMacro[i].isPlayer1) { lastP1Input = &cbfMacro[i]; }
                    if (!lastP2Input && !cbfMacro[i].isPlayer1) { lastP2Input = &cbfMacro[i]; }
                    if (lastP1Input && lastP2Input) break;
                }

                bool p1Holding = lastP1Input && lastP1Input->isHolding;
                bool p2Holding = lastP2Input && lastP2Input->isHolding;

                std::erase_if(m_queuedButtons, [&](const auto& b) { return b.m_isPush == (b.m_isPlayer2 ? p2Holding : p1Holding); });

                if (cbfMacro.empty() && !m_queuedButtons.empty()) { std::erase_if(m_queuedButtons, [](const auto& b) { return !b.m_isPush; }); }
            }
        }
    }
};

void addInput(unsigned long long frame, bool isHolding, bool isPlayer1)
{
    if (!cbf) { macro.push_back({frame, isHolding, isPlayer1}); }
    else { cbfMacro.push_back({float(frame), isHolding, isPlayer1}); }
}

void simulateInput(bool isHolding, bool isPlayer1)
{
    auto playLayer = PlayLayer::get();
    if (!playLayer) { return; }
    playLayer->queueButton(static_cast<int>(PlayerButton::Jump), isHolding, !isPlayer1, playLayer->m_timestamp);
}
