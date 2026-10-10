#include "autoclicker.hpp"
#include "../bot/replay.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(AutoClickerGJBaseGameLayer, GJBaseGameLayer)
{
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        if (player1 && replayStatus != 2)
        {
            if (counterP1 == 0)
            {
                GJBaseGameLayer::handleButton(true, static_cast<int>(PlayerButton::Jump), true);
                if (p1Swift)
                {
                    GJBaseGameLayer::handleButton(false, static_cast<int>(PlayerButton::Jump), true);
                }
            }
            if (counterP1 == p1HoldFor)
            {
                if (p1Swift)
                {
                    GJBaseGameLayer::handleButton(true, static_cast<int>(PlayerButton::Jump), true);
                }
                GJBaseGameLayer::handleButton(false, static_cast<int>(PlayerButton::Jump), true);
                counterP1 = -p1ReleaseFor;
            }
            counterP1++;
        }
        if (player2 && replayStatus != 2)
        {
            if (counterP2 == 0)
            {
                GJBaseGameLayer::handleButton(true, static_cast<int>(PlayerButton::Jump), false);
                if (p2Swift)
                {
                    GJBaseGameLayer::handleButton(false, static_cast<int>(PlayerButton::Jump), false);
                }
            }
            if (counterP2 == p2HoldFor)
            {
                if (p2Swift)
                {
                    GJBaseGameLayer::handleButton(true, static_cast<int>(PlayerButton::Jump), false);
                }
                GJBaseGameLayer::handleButton(false, static_cast<int>(PlayerButton::Jump), false);
                counterP2 = -p2ReleaseFor;
            }
            counterP2++;
        }
    }
};