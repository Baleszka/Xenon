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

// Practice fix and macro recording logic
// Genuinely terrible code don't do ts
class $modify(ReplayPlayLayer, PlayLayer)
{
    void resetLevel()
    {
        if (m_playerDied && replayStatus == 0 || replayStatus == 1)
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

        // Save last queued button before reset for 2p bugfix
        std::vector<PlayerButtonCommand> m_previouslyQueuedButtons = m_queuedButtons;

        PlayLayer::resetLevel();

        if (replayStatus == 0)
        {
            if (!cbf)
            {
                // Two player bugfix
                if (this->m_level->m_twoPlayerMode && m_gameState.m_isDualMode)
                {
                    // in 2 player mode if reset with both players released, it only queues one players release
                    // this also queues the other one
                    if (!m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush && m_previouslyQueuedButtons.empty())
                    {
                        queueButton(static_cast<int>(PlayerButton::Jump), false, !m_queuedButtons.back().m_isPlayer2, m_timestamp);
                    }
                    
                    if (macro.size() >= 2)
                    {
                        input* lastP1Input = nullptr; input* lastP2Input = nullptr;

                        for (size_t i = macro.size(); i-- > 0; )
                        {
                            if (!lastP1Input && macro[i].isPlayer1)  lastP1Input = &macro[i];
                            if (!lastP2Input && !macro[i].isPlayer1) lastP2Input = &macro[i];
                            if (lastP1Input && lastP2Input) break;
                        }

                        // Tn 2 player mode if reset with ONE player released, it queues a p1 push and nothing else
                        // This queues a release for the player which released before resetting
                        if (lastP1Input && lastP2Input && lastP1Input->isHolding != lastP2Input->isHolding && m_previouslyQueuedButtons.empty())
                        {
                            m_queuedButtons.clear();
                            if (!lastP1Input->isHolding) { queueButton(static_cast<int>(PlayerButton::Jump), false, false, m_timestamp); }
                            if (!lastP2Input->isHolding) { queueButton(static_cast<int>(PlayerButton::Jump), false, true, m_timestamp); }
                        }

                        // In 2 player mode if reset with both players released but one makes an input without advancing any frames
                        // both go up for some reason
                        // This adds a release for the player which didnt click
                        if (lastP1Input && lastP2Input && lastP1Input->isHolding == lastP2Input->isHolding && !m_previouslyQueuedButtons.empty() && m_previouslyQueuedButtons.back().m_isPush)
                        {
                            m_queuedButtons.clear();
                            queueButton(static_cast<int>(PlayerButton::Jump), false, !m_previouslyQueuedButtons.back().m_isPlayer2, m_timestamp);
                        }
                        
                        // If one player was going up and the other started holding but didnt advance any frames before resetting
                        if (!m_previouslyQueuedButtons.empty() && m_previouslyQueuedButtons.back().m_isPush && lastP1Input && lastP2Input && lastP1Input->isHolding != lastP2Input->isHolding)
                        {
                            m_queuedButtons.clear();
                        }

                        // If one player was going up and the other started holding and also advanced one or more frames before resetting
                        if (m_previouslyQueuedButtons.empty() && lastP1Input && lastP2Input && lastP1Input->isHolding == lastP2Input->isHolding && lastP1Input->isHolding)
                        {
                            m_queuedButtons.clear();
                        }
                    }
                }
                
                // remove inputs if the frame they were made on is bigger than the frame reset to
                while (!macro.empty() && macro.back().frame >= getFrame()) { macro.pop_back(); }

                // if last input is a release and queuedButtons' last element is also a release, clear queuedButtons so it doesn't add an unnecessary release
                if (!m_queuedButtons.empty() && !macro.empty() && !macro.back().isHolding && !m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
                
                // If you're a game mode whos release matters and you're holding while you reset, it doesn't start holding again, rather it ignores it and only adds the release when you release.
                // This is for gamemodes like wave, ship and robot.
                // (Technically UFO release matters too because if you do a swift while UFO nothing happens lel)
                // Don't do this in two player dual mode because it breaks things.
                if (!(this->m_level->m_twoPlayerMode && m_gameState.m_isDualMode) && !m_queuedButtons.empty() && !macro.empty() && macro.back().isHolding && m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }

                // If you reset to a frame that was before any inputs were made and queuedButtons has a release input, ignore it.
                // This fixes ghost release.
                if (macro.empty() && !m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
            }
            else
            {
                if (this->m_level->m_twoPlayerMode && m_gameState.m_isDualMode)
                {
                    if (!m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush && m_previouslyQueuedButtons.empty())
                    {
                        queueButton(static_cast<int>(PlayerButton::Jump), false, !m_queuedButtons.back().m_isPlayer2, m_timestamp);
                    }
                    
                    if (cbfMacro.size() >= 2)
                    {
                        cbfInput* lastP1Input = nullptr; cbfInput* lastP2Input = nullptr;

                        for (size_t i = cbfMacro.size(); i-- > 0; )
                        {
                            if (!lastP1Input && cbfMacro[i].isPlayer1)  lastP1Input = &cbfMacro[i];
                            if (!lastP2Input && !cbfMacro[i].isPlayer1) lastP2Input = &cbfMacro[i];
                            if (lastP1Input && lastP2Input) break;
                        }

                        if (lastP1Input && lastP2Input && lastP1Input->isHolding != lastP2Input->isHolding && m_previouslyQueuedButtons.empty())
                        {
                            m_queuedButtons.clear();
                            if (!lastP1Input->isHolding) { queueButton(static_cast<int>(PlayerButton::Jump), false, false, m_timestamp); }
                            if (!lastP2Input->isHolding) { queueButton(static_cast<int>(PlayerButton::Jump), false, true, m_timestamp); }
                        }

                        if (lastP1Input && lastP2Input && lastP1Input->isHolding == lastP2Input->isHolding && !m_previouslyQueuedButtons.empty() && m_previouslyQueuedButtons.back().m_isPush)
                        {
                            m_queuedButtons.clear();
                            queueButton(static_cast<int>(PlayerButton::Jump), false, !m_previouslyQueuedButtons.back().m_isPlayer2, m_timestamp);
                        }
                        
                        if (!m_previouslyQueuedButtons.empty() && m_previouslyQueuedButtons.back().m_isPush && lastP1Input && lastP2Input && lastP1Input->isHolding != lastP2Input->isHolding)
                        {
                            m_queuedButtons.clear();
                        }

                        if (m_previouslyQueuedButtons.empty() && lastP1Input && lastP2Input && lastP1Input->isHolding == lastP2Input->isHolding && lastP1Input->isHolding)
                        {
                            m_queuedButtons.clear();
                        }
                    }
                }

                while (!cbfMacro.empty() && cbfMacro.back().preciseFrame >= getFrame()) { cbfMacro.pop_back(); }

                if (!cbfMacro.empty() && !m_queuedButtons.empty() && !cbfMacro.back().isHolding && !m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
                if (!cbfMacro.empty() && !m_queuedButtons.empty() && cbfMacro.back().isHolding && m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
                if (cbfMacro.empty() && !m_queuedButtons.empty() && !m_queuedButtons.back().m_isPush) { m_queuedButtons.clear(); }
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
