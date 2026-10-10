#include "releaseFix.hpp"

using namespace geode::prelude;

bool patchesEnabled = false;

void applyPatches()
{
    for (auto* p : Mod::get()->getPatches()) {
        if (!p->isEnabled()) (void)p->enable();
    }
    geode::log::info("Enabled patches");
    patchesEnabled = true;
}

void disablePatches()
{
    for (auto* p : Mod::get()->getPatches()) {
        if (p->isEnabled()) (void)p->disable();
    }
    geode::log::info("Disabled patches");
    patchesEnabled = false;
}