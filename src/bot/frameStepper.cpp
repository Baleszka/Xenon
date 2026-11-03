#include "frameStepper.hpp"
#include "tpsBypass.hpp"

#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

$on_game(Loaded) 
{
    listenForKeybindSettingPresses("framestepper-toggle", [](Keybind const& keybind, bool down, bool repeat, double timestamp) 
    {
        if (down && !repeat && replayStatus != 2 && PlayLayer::get()) { frameStepper = !frameStepper; }
    });

    listenForKeybindSettingPresses("framestepper-step", [](Keybind const& keybind, bool down, bool repeat, double timestamp) 
    {
        if (down && !repeat && frameStepper) { shouldStep = true; }
        else if (down && repeat && frameStepper) { shouldStep = true; }
    });
}

class $modify(FSPlayLayer, PlayLayer)
{
    void onQuit() 
    {
        frameStepper = false;
        PlayLayer::onQuit();
    }
};

class $modify(FSCCScheduler, CCScheduler)
{
    void update(float dt)
    {
        if (!frameStepper || isPaused()) { CCScheduler::update(dt); }
        else if (frameStepper && shouldStep) { shouldStep = false; CCScheduler::update(dt); }
    }
};
