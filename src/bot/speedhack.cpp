#include "speedhack.hpp"
#include "replay.hpp"
#include "tpsBypass.hpp"
#include "frameStepper.hpp"

#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

class $modify(SpeedhackCCScheduler, cocos2d::CCScheduler)
{
    void update(float dt)
    {
        if (replayStatus == 2)
        {
            lockDeltaOverflow = 0.0;
            lockDeltaStep = false;
            CCScheduler::update(dt);
            return;
        }

        dt *= speedhack;

        auto pl = PlayLayer::get();
        if (!lockDelta || !pl || pl->m_isPaused)
        {
            lockDeltaOverflow = 0.0;
            CCScheduler::update(dt);
            return;
        }

        if (frameStepper)
        {
            lockDeltaOverflow = 0.0;
            lockDeltaStep = true;
            CCScheduler::update(dt);
            lockDeltaStep = false;
            return;
        }

        const double stepDt = getPhysicsDt();
        const double timeWarp = pl->m_gameState.m_timeWarp;
        lockDeltaOverflow += static_cast<double>(dt) * (timeWarp > 1.0 ? timeWarp : 1.0);

        int steps = static_cast<int>(std::floor(lockDeltaOverflow / stepDt));
        const int maxSteps = std::max(1, tps / 30);
        if (steps > maxSteps)
        {
            steps = maxSteps;
            lockDeltaOverflow = 0.0;
        }
        else
        {
            lockDeltaOverflow -= steps * stepDt;
        }

        lockDeltaStep = true;
        for (int i = 0; i < steps; i++)
        {
            CCScheduler::update(static_cast<float>(stepDt));

            auto current = PlayLayer::get();
            if (!current || current->m_isPaused)
            {
                lockDeltaOverflow = 0.0;
                break;
            }
        }
        lockDeltaStep = false;
    }
};

class $modify(SpeedhackAudioFMODAudioEngine, FMODAudioEngine)
{
    void update(float dt) 
    {
        FMODAudioEngine::update(dt);
        if (speedhackAudio && replayStatus != 2)
        {
            FMOD::ChannelGroup* group;
            if (m_system->getMasterChannelGroup(&group) == FMOD_OK) { group->setPitch(speedhack); } // speedhack audio
        }
    }
};
