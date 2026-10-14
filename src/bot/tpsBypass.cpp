#include "tpsBypass.hpp"
#include "midhook.hpp"
#include "replay.hpp"

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <cmath>

using namespace geode::prelude;

float getTimeWarp()
{
    if (auto pl = PlayLayer::get()) { return pl->m_gameState.m_timeWarp; }
    return 1.0;
}

void calculateSteps(float dt, float targetDt)
{
    dt += m_tpsOverflow;

    float wantedDt = targetDt * fminf(getTimeWarp(), 1.0f);
    if (wantedDt == 0.0f) { return; }

    int steps = std::floor(dt / wantedDt);
    
    m_tpsOverflow = dt - steps * wantedDt;

    if (isPaused())
    {
        steps = 1;
        m_tpsOverflow = 0.0f;
        if (auto pl = PlayLayer::get()) { pl->m_extraDelta = 0.0f; }
    }

    totalStepCount = steps;
    estimatedStepCount = steps;
}

void physDtMidhook(SafetyHookContext &ctx)
{
    auto pl = GJBaseGameLayer::get();
    if (!pl) { return; }

    if (replayStatus == 2) { return; }

    ctx.xmm1.f64[0] *= tps / 60.0;
    ctx.rip += 0x08;
}

void physStepCountMidhook(SafetyHookContext &ctx)
{
    if (replayStatus == 2) { return; }
        
    ctx.rdx = 2 - estimatedStepCount;
}

void restorePhysDtHook(SafetyHookContext &ctx)
{
    auto pl = GJBaseGameLayer::get();
    if (!pl) { return; }

    if (replayStatus == 2) { return; }

    double newDt = getPhysicsDt() * estimatedStepCount;
    ctx.xmm9.f64[0] = newDt;
}

class $modify(TPSBypassGameLayer, GJBaseGameLayer)
{
    double getModifiedDelta(float dt)
    {
        if (replayStatus == 2) { return GJBaseGameLayer::getModifiedDelta(dt); }

        double modifiedDelta = GJBaseGameLayer::getModifiedDelta(dt);
        if (auto editor = LevelEditorLayer::get(); editor && editor->m_playbackActive) {return modifiedDelta;}

        return getPhysicsDt() * fminf(m_gameState.m_timeWarp, 1.0f);
    }

    void update(float dt)
    {
        if (replayStatus == 2)
        {
            m_tpsOverflow = 0.0;
            GJBaseGameLayer::update(dt);
            return;
        }

        calculateSteps(dt, static_cast<float>(getPhysicsDt()));
        if (estimatedStepCount > 0) { GJBaseGameLayer::update(dt); }
    }
};

$execute
{
    util::midhook(geode::base::get() + 0x237A7C, "physDt", physDtMidhook);
    util::midhook(geode::base::get() + 0x237DCE, "physStepCount", physStepCountMidhook);
    util::midhook(geode::base::get() + 0x238F6E, "restorePhysDt", restorePhysDtHook);
}

// Borrowed from silicate :3
// thanks nat for gplv3 <3