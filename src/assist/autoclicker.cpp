#include "autoclicker.hpp"
#include "../bot/replay.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(AutoClickerGJBaseGameLayer, GJBaseGameLayer)
{
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        
        if (player1 && replayStatus != 2)
        {
            if (counterP1 == 0)
            {
                queueButton(static_cast<int>(PlayerButton::Jump), true, false, m_timestamp);
                if (p1Swift)
                {
                    queueButton(static_cast<int>(PlayerButton::Jump), false, false, m_timestamp);
                }
            }
            if (counterP1 == p1HoldFor)
            {
                if (p1Swift)
                {
                    queueButton(static_cast<int>(PlayerButton::Jump), true, false, m_timestamp);
                }
                queueButton(static_cast<int>(PlayerButton::Jump), false, false, m_timestamp);
                counterP1 = -p1ReleaseFor;
            }
            counterP1++;
        }
        if (player2 && replayStatus != 2)
        {
            if (counterP2 == 0)
            {
                queueButton(static_cast<int>(PlayerButton::Jump), true, true, m_timestamp);
                if (p2Swift)
                {
                    queueButton(static_cast<int>(PlayerButton::Jump), false, true, m_timestamp);
                }
            }
            if (counterP2 == p2HoldFor)
            {
                if (p2Swift)
                {
                    queueButton(static_cast<int>(PlayerButton::Jump), true, true, m_timestamp);
                }
                queueButton(static_cast<int>(PlayerButton::Jump), false, true, m_timestamp);
                counterP2 = -p2ReleaseFor;
            }
            counterP2++;
        }
        // call processCommands after so it doesn't wait an extra frame
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};