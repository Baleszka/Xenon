#include "savePreferences.hpp"
#include "../assist/hitboxes.hpp"
#include "../bot/speedhack.hpp"
#include "../bot/replay.hpp"

#include <Geode/modify/MenuLayer.hpp>
#include <algorithm>

using namespace geode::prelude;

class $modify(MenuLayer)
{
    void onQuit(CCObject* sender)
    {
        hitboxSaveData hitboxSave;

        hitboxSave.hitboxWidth = hitboxWidth;
        hitboxSave.trailLength = trailLength;

        std::copy(std::begin(hazardCol), std::end(hazardCol), hitboxSave.hazardCol.begin());
        std::copy(std::begin(solidCol), std::end(solidCol), hitboxSave.solidCol.begin());
        std::copy(std::begin(interactableCol), std::end(interactableCol), hitboxSave.interactableCol.begin());
        std::copy(std::begin(hazardRotCol), std::end(hazardRotCol), hitboxSave.hazardRotCol.begin());
        std::copy(std::begin(passableCol), std::end(passableCol), hitboxSave.passableCol.begin());
        
        hitboxSave.showPlayer = showPlayer;
        hitboxSave.showPlayerRot = showPlayerRot;
        hitboxSave.showPlayerInner = showPlayerInner;
        hitboxSave.showPlayerCircle = showPlayerCircle;

        Mod::get()->setSavedValue<hitboxSaveData>("hitboxData", hitboxSave);

        replaySaveData replaySave;

        replaySave.speedhackAudio = speedhackAudio;
        replaySave.blockInputs = blockInputs;

        Mod::get()->setSavedValue<replaySaveData>("replayData", replaySave);

        MenuLayer::onQuit(sender);
    }

    bool init()
    {
        if (!MenuLayer::init()) return false;

        auto hitboxLoad = Mod::get()->getSavedValue<hitboxSaveData>("hitboxData");

        hitboxWidth = hitboxLoad.hitboxWidth;
        trailLength = hitboxLoad.trailLength;

        std::copy(hitboxLoad.hazardCol.begin(), hitboxLoad.hazardCol.end(), hazardCol);
        std::copy(hitboxLoad.solidCol.begin(), hitboxLoad.solidCol.end(), solidCol);
        std::copy(hitboxLoad.interactableCol.begin(), hitboxLoad.interactableCol.end(), interactableCol);
        std::copy(hitboxLoad.hazardRotCol.begin(), hitboxLoad.hazardRotCol.end(), hazardRotCol);
        std::copy(hitboxLoad.passableCol.begin(), hitboxLoad.passableCol.end(), passableCol);

        showPlayer = hitboxLoad.showPlayer;
        showPlayerRot = hitboxLoad.showPlayerRot;
        showPlayerInner = hitboxLoad.showPlayerInner;
        showPlayerCircle = hitboxLoad.showPlayerCircle;

        auto replayLoad = Mod::get()->getSavedValue<replaySaveData>("replayData");

        speedhackAudio = replayLoad.speedhackAudio;
        blockInputs = replayLoad.blockInputs;

        return true;
    }
};