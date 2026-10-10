#include "frameStepper.hpp"
#include "replay.hpp"

#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(FSKeybind, CCKeyboardDispatcher)
{
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double ts)
    {
        if (key == enumKeyCodes::KEY_V && isKeyDown && replayStatus != 2) { frameStepper = !frameStepper; }
        if (key == enumKeyCodes::KEY_C && isKeyDown && frameStepper) { shouldStep = true; }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, ts);
    }
};

class $modify(FSGJBaseGameLayer, GJBaseGameLayer)
{
    void update(float dt)
    {
        if (replayStatus != 2)
        {
            if (!frameStepper) { GJBaseGameLayer::update(dt); } // update normally
            else if (frameStepper && shouldStep) { shouldStep = false; GJBaseGameLayer::update( 1.0f / float(tps) ); m_extraDelta = 0.f; } // step the proper amount (1.0 / tps -> deltatime)
        }
        else { GJBaseGameLayer::update(dt); }
    }
};
