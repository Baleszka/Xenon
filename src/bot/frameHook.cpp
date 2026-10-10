#include "frameHook.hpp"
#include "../checkpoint/checkpoint.hpp"
#include "tpsBypass.hpp"

#include <Geode/modify/PlayLayer.hpp>
#include <safetyhook.hpp>

using namespace geode::prelude;

static SafetyHookMid g_frameHook;

static void frameUpdateMidhook(SafetyHookContext&) 
{
    auto pl = PlayLayer::get();
    if (!pl) return;

    if (!pl->m_playerDied) {
        g_frame++;
    }
}

$execute 
{
    g_frameHook = safetyhook::create_mid(
        reinterpret_cast<void*>(geode::base::get() + 0x238BAA),
        frameUpdateMidhook
    );
}

class $modify(FrameResetHook, PlayLayer) 
{
    void resetLevel() 
    {
        g_frame = 0;
        PlayLayer::resetLevel();
    }
};

int getFrame() 
{
    if (!PlayLayer::get()) return 0;
    return g_frame;
}

// borrowed from silicate code could not figure ts out by myself :sob:
// credits to peony