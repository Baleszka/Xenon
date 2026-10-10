#include "speedhack.hpp"
#include "replay.hpp"

#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

using namespace geode::prelude;

class $modify(SpeedhackCCScheduler, cocos2d::CCScheduler)
{
    void update(float dt)
    {
        if (replayStatus != 2) { dt *= speedhack; } // speedhack game
        CCScheduler::update(dt);
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