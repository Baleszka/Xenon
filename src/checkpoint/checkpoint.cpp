#include "checkpoint.hpp"
#include "../bot/frameHook.hpp"

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(CheckpointPlayLayer, PlayLayer) 
{
    CheckpointObject* createCheckpoint() 
    {
        CheckpointObject* ret = PlayLayer::createCheckpoint();
        checkPoints.push_back({static_cast<int>(g_frame), ret});
        return ret;
    }
    void removeAllCheckpoints() 
    {
        PlayLayer::removeAllCheckpoints();
        checkPoints.clear();
    }
    void removeCheckpoint(bool first) 
    {
        PlayLayer::removeCheckpoint(first);
        checkPoints.pop_back();
    }
    void loadFromCheckpoint(CheckpointObject* object) 
    {
        PlayLayer::loadFromCheckpoint(object);
        g_frame = checkPoints.back().frame; // get frame of last checkpoint
        g_frame++; // idk why but it fixes frame desync lel
    }
};