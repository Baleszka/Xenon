#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace phys
{
    bool hasBeenActivatedByPlayer(PlayerObject* player, EnhancedGameObject* object);
    void activateForTrajectory(EffectGameObject* object, PlayerObject* player);

    void flipGravity(PlayerObject* player, bool gravity);
    void propellPlayer(PlayerObject* player, float force, bool playEffect, int objectType);
    void bumpPlayer(PlayerObject* player, float force, int objectType, bool playEffect, GameObject* object);

    void teleportPlayer(GJBaseGameLayer* pl, TeleportPortalObject* object, PlayerObject* player);

    void startDashing(PlayerObject* player, DashRingObject* ring);
    void stopDashing(PlayerObject* player);
    void ringJump(PlayerObject* player, RingObject* ring);
    void togglePlayerScale(PlayerObject* player, bool smallSize);

    void collisionCheckObjects(GJBaseGameLayer* pl, PlayerObject* player, gd::vector<GameObject*>* objects, int objectCount, float dt);
    void triggerObject(EffectGameObject* object, GJBaseGameLayer* pl, PlayerObject* player);
    void checkSpawnObjects(GJBaseGameLayer* pl, PlayerObject* player);
}
