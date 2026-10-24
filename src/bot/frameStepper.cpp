#include "frameStepper.hpp"
#include "replay.hpp"

#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

$on_game(Loaded) {
    listenForKeybindSettingPresses("framestepper-toggle", [](Keybind const& keybind, bool down, bool repeat, double timestamp) {
        if (down && !repeat && replayStatus != 2)
        {
            frameStepper = !frameStepper;
        }
    });
    listenForKeybindSettingPresses("framestepper-step", [](Keybind const& keybind, bool down, bool repeat, double timestamp) {
        if (down && !repeat && frameStepper)
        {
            shouldStep = true;
        }
        else if (down && repeat && frameStepper)
        {
            shouldStep = true;
        }
    });
}

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
