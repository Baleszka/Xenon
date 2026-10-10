#include "trajectoryPhysics.hpp"
#include "trajectory.hpp"

#include <cmath>

using namespace geode::prelude;

namespace phys
{
    static bool activatedPlatformer(EnhancedGameObject* object, bool isPlatformer)
    {
        if (!isPlatformer)
        {
            if (object->m_isMultiActivate) { return true; }
        }
        else if (object->m_isNoMultiActivate) { return true; }

        return false;
    }

    bool hasBeenActivatedByPlayer(PlayerObject* player, EnhancedGameObject* object)
    {
        if (activatedPlatformer(object, player->m_isPlatformer)) { return false; }

        if (!player->m_isSecondPlayer) { return object->m_activatedByPlayer1; }

        return object->m_activatedByPlayer2;
    }

    void activateForTrajectory(EffectGameObject* object, PlayerObject* player)
    {
        TrajectoryManager::get().rememberActivatedObject(object, player);
    }

    static void flipGravityInner(PlayerObject* player, bool gravity)
    {
        player->m_isUpsideDown = gravity;

        player->m_lastFlipTime = player->m_totalTime;
        player->m_collidedBottomMaxY = 0.;
        player->m_collidedTopMinY = 0.;
        player->m_unkA29 = false;
        if (player->m_wasOnSlope || player->m_isOnSlope)
        {
            player->m_slopeFlipGravityRelated = !player->m_slopeFlipGravityRelated;
        }

        player->m_collisionLogTop->removeAllObjects();
        player->m_collisionLogBottom->removeAllObjects();
        player->m_collisionLogLeft->removeAllObjects();
        player->m_collisionLogRight->removeAllObjects();

        player->m_unk50C = -1;
        player->m_unk510 = -1;
        player->m_lastCollisionBottom = -1;
        player->m_lastCollisionTop = -1;
        player->m_lastCollisionLeft = -1;
        player->m_lastCollisionRight = -1;

        player->m_yVelocity = player->m_yVelocity * 0.5;
        player->m_isOnGround = false;
        if (player->m_isBall)
        {
            player->m_isRotating = false;
            player->m_isBallRotating = false;
            player->m_isBallRotating2 = false;
            player->m_rotationSpeed = 0.;
            player->runBallRotation2();
        }
    }

    void flipGravity(PlayerObject* player, bool gravity)
    {
        if (player->m_isUpsideDown == gravity) { return; }

        auto gameLayer = GJBaseGameLayer::get();
        auto& gameState = gameLayer->m_gameState;
        auto& trajectory = TrajectoryManager::get();

        flipGravityInner(player, gravity);
        if (trajectory.isFakePlayer(player) && !gameState.m_unkBool31 && gameState.m_isDualMode && !gameLayer->m_levelSettings->m_twoPlayerMode)
        {
            auto p1 = player;
            auto p2 = trajectory.getOtherPlayer(player);
            if (!p2) { return; }

            if (!(p1->m_isShip == p2->m_isShip && p1->m_isBall == p2->m_isBall &&
                  p1->m_isBird == p2->m_isBird && p1->m_isSpider == p2->m_isSpider &&
                  p1->m_isRobot == p2->m_isRobot && p1->m_isSwing == p2->m_isSwing))
            {
                return;
            }

            flipGravityInner(p2, !gravity);
        }
    }

    void propellPlayer(PlayerObject* player, float force, bool, int)
    {
        player->m_maybeIsBoosted = true;
        player->m_isOnGround2 = false;
        player->m_isOnGround = false;
        player->m_isOnSlope = false;
        player->m_wasOnSlope = false;

        float gravity = player->m_vehicleSize != 1.0 ? 0.8 : 1.0;
        float gravityCoefficient = player->m_isUpsideDown ? -1 : 1;
        double velocity = force * gravity * gravityCoefficient * 16.0;
        player->setYVelocity(velocity, 0);

        if (player->m_isBall || player->m_isSpider || player->m_isSwing)
        {
            player->m_yVelocity *= 0.6;
        }

        if (!player->m_isLocked && !player->m_isDashing)
        {
            player->m_isRotating = false;
            player->m_isBallRotating = false;
            player->m_isBallRotating2 = false;
            player->m_rotationSpeed = 0.;

            if (!player->m_isBall) { player->runBallRotation(velocity); }
            else { player->runNormalRotation(0, 1.0); }
        }

        player->m_lastGroundedPos = player->getPosition();
    }

    void bumpPlayer(PlayerObject* player, float force, int objectType, bool playEffect, GameObject* object)
    {
        if (player->m_isPlatformer || !player->m_fixRobotJump)
        {
            player->m_touchedPad = true;
        }

        if (objectType != 0x2c)
        {
            propellPlayer(player, force, playEffect, objectType);
            player->m_isAccelerating = false;
            if (objectType == 0x22)
            {
                player->m_isAccelerating = true;
                player->m_lastGroundedPos = CCPoint{0.0, 0.0};
            }

            return;
        }

        if (!object)
        {
            player->spiderTestJumpInternal(false);
            return;
        }

        if (!player->m_isSideways)
        {
            if (player->m_isUpsideDown != object->isFacingDown())
            {
                flipGravity(player, !player->m_isUpsideDown);
            }
        }
        else
        {
            if (player->m_isUpsideDown != object->isFacingLeft())
            {
                flipGravity(player, !player->m_isUpsideDown);
            }
        }

        player->spiderTestJumpInternal(false);
    }

    static cocos2d::CCArray* getGroup(GJBaseGameLayer* pl, int groupID)
    {
        if (groupID < 0) { groupID = 0; }
        if (groupID > 9999) { groupID = 9999; }

        cocos2d::CCArray* group = pl->m_groups.at(groupID);
        if (!group)
        {
            group = cocos2d::CCArray::create();
            pl->m_groupDict->setObject(group, groupID);
            pl->m_groups.at(groupID) = group;
        }

        return group;
    }

    static float redirectPlayerForce(PlayerObject* player, float force)
    {
        cocos2d::CCPoint velocityCoords = cocos2d::CCPoint{(float)player->m_platformerXVelocity, (float)player->m_yVelocity};

        cocos2d::CCPoint p;

        float angle = force * 0.017453292 - atan2f(velocityCoords.y, velocityCoords.x);
        if (angle != 0.0)
        {
            float s = sin(angle);
            float c = cos(angle);

            p = cocos2d::CCPoint{
                (float)(velocityCoords.x * s) - (float)(velocityCoords.y * c),
                (float)(velocityCoords.x * c) + (float)(velocityCoords.y * s)};
        }

        velocityCoords = p;

        if (player->m_isSideways)
        {
            float x = velocityCoords.x;
            velocityCoords.x = velocityCoords.y;
            velocityCoords.y = x;
        }

        player->m_yVelocity = velocityCoords.y;
        player->m_isAccelerating = true;
        if (player->m_isPlatformer)
        {
            player->m_platformerXVelocity = velocityCoords.x;
            player->m_affectedByForces = true;
        }

        return player->m_yVelocity;
    }

    void teleportPlayer(GJBaseGameLayer* pl, TeleportPortalObject* object, PlayerObject* player)
    {
        if (!player || !object) { return; }

        player->m_wasTeleported = true;

        if (object->m_orangePortal)
        {
            CCPoint bluePortalStartPos = object->getStartPos();
            CCPoint orangePortalStartPos = object->m_orangePortal->getStartPos();
            object->m_teleportYOffset = orangePortalStartPos.y - bluePortalStartPos.y;
        }

        cocos2d::CCPoint playerPos = player->getPosition();
        TeleportPortalObject* destination = object->m_orangePortal;
        if (!destination)
        {
            int targetGroupID = object->m_targetGroupID;
            if (targetGroupID > 0)
            {
                cocos2d::CCArray* group = getGroup(pl, targetGroupID);
                int groupLength = group->count();

                float random;

                if (groupLength > 0)
                {
                    if (groupLength == 1)
                    {
                        random = 0.0;
                    }
                    else
                    {
                        random = (float)rand() / (float)RAND_MAX;
                        if (random == 1.0) { random = 0.0; }
                    }

                    destination = (TeleportPortalObject*)group->objectAtIndex((int)(random * groupLength));
                }
            }
        }

        if (destination)
        {
            CCPoint destinationPos;
            CCPoint portalPos = object->getPosition();
            player->m_lastPortalPos = object->getPosition();
            player->m_lastActivatedPortal = object;
            player->m_fallStartY = 0.0;
            if (object->m_objectID == 0x2eb)
            {
                float dy = object->m_teleportYOffset;
                portalPos = object->getRealPosition();

                destinationPos = cocos2d::CCPoint{player->getPosition().x, portalPos.y + dy};
            }
            else
            {
                destinationPos = destination->getPosition();
            }
            if (object->m_saveOffset)
            {
                portalPos = object->getRealPosition();

                portalPos -= playerPos;
                destinationPos -= portalPos;
            }
            if (object->m_ignoreX)
            {
                destinationPos.x = playerPos.x;
            }
            if (object->m_ignoreY)
            {
                destinationPos.y = playerPos.y;
            }
            player->setPosition(destinationPos);
        }

        if (object->m_gravityMode > 0)
        {
            bool gravity = false;

            if (object->m_gravityMode == 2) { gravity = true; }
            else if (object->m_gravityMode == 3) { gravity = !player->m_isUpsideDown; }

            flipGravity(player, gravity);
        }

        float forceRelated;
        if (destination)
        {
            float gravityForceMod = 90.0;
            int objectID = object->m_objectID;
            if (objectID == 0x26 || objectID == 0x2eb || objectID == 0x2ed || objectID == 0x810 || objectID == 0xb56)
            {
                gravityForceMod = 180.0;
            }

            float flipMod = (float)(object->isFlipX() ? 0xb4 : 1);
            forceRelated = flipMod + (gravityForceMod - destination->getRotationX());
        }
        else
        {
            float flipMod = (float)(object->isFlipX() ? 0xb4 : 1);
            forceRelated = flipMod - object->getRotationX();
        }

        if (object->m_redirectForceEnabled)
        {
            redirectPlayerForce(player, forceRelated);
        }
        else if (object->m_staticForceEnabled)
        {
            float force = object->m_staticForce;
            if (force == 0.0 && !object->m_staticForceAdditive)
            {
                player->m_isAccelerating = false;
                player->m_yVelocity = 0.0;
                if (player->m_isPlatformer)
                {
                    player->m_platformerXVelocity = 0.0;
                    player->m_affectedByForces = false;
                }
            }
            else
            {
                float forceMod = forceRelated * 0.01745329;
                cocos2d::CCPoint p = CCPoint{(float)std::cos(forceMod), (float)std::sin(forceMod)};

                float length = p.getLength();
                if (length > 0.0)
                {
                    cocos2d::CCPoint forceVector = p * (force / length);
                    if (player->m_isSideways)
                    {
                        float x = forceVector.x;
                        forceVector.x = forceVector.y;
                        forceVector.y = x;
                    }

                    player->m_isAccelerating = true;
                    double newYVelocity = forceVector.y;
                    if (object->m_staticForceAdditive)
                    {
                        newYVelocity += player->m_yVelocity;
                    }
                    player->m_yVelocity = newYVelocity;
                    if (player->m_isPlatformer)
                    {
                        double newXVelocity = forceVector.x;
                        if (object->m_staticForceAdditive)
                        {
                            newXVelocity += player->m_platformerXVelocity;
                        }
                        player->m_platformerXVelocity = newXVelocity;
                        player->m_affectedByForces = true;
                    }
                }
            }
        }

        if (object->m_snapGround)
        {
            player->m_lastGroundedPos = player->getPosition();
        }
    }

    void startDashing(PlayerObject* player, DashRingObject* ring)
    {
        if (player->m_isDashing) { return; }

        player->m_isDashing = true;
        player->m_lastLandTime = 0.;
        player->m_isRotating = false;
        player->m_isBallRotating2 = false;
        player->m_isBallRotating = false;
        player->m_rotationSpeed = 0.;

        player->m_dashRing = ring;
        player->m_dashStartTime = player->m_totalTime;
        float dashAngle = ring->getObjectRotation();
        if (ring->isFlipX())
        {
            dashAngle += 180.0;
        }

        if (fabs((ring->getRotationX() - ring->getRotationY())) > 179.0)
        {
            dashAngle += 180.0;
        }
        dashAngle = fmodf(-dashAngle, 360.0);

        if (dashAngle < -180.0) { dashAngle += 360.0; }
        else if (dashAngle > 180.0) { dashAngle -= 360.0; }

        float sidewaysAngle = 0.0;
        if (player->m_isPlatformer)
        {
            sidewaysAngle = ring->m_dashSpeed * 5.77;
        }
        else
        {
            if (player->m_isSideways)
            {
                sidewaysAngle = -90.0;
            }
            if (player->m_isGoingLeft)
            {
                sidewaysAngle += 180.0;
            }

            float angleHelper = dashAngle;
            if (sidewaysAngle != 0)
            {
                angleHelper = dashAngle + sidewaysAngle;
                if (angleHelper < -180.0) { angleHelper += 360.0; }
                else if (angleHelper > 180.0) { angleHelper -= 360.0; }
            }

            if (fabs(angleHelper) > 90.0)
            {
                if (angleHelper <= 0.0) { dashAngle = -180.0; }
                else { dashAngle = 180.0; }

                angleHelper = dashAngle - angleHelper;
                if (angleHelper < -180.0) { angleHelper += 360.0; }
                else if (angleHelper > 180.0) { angleHelper -= 360.0; }
            }

            float angleHelper2 = 70.0;
            dashAngle = angleHelper;
            if (angleHelper > 70.0 || angleHelper < -70.0)
            {
                dashAngle = angleHelper2;

                if (angleHelper <= 0.0)
                {
                    if ((angleHelper < 70.0) && (angleHelper > -180.0))
                    {
                        dashAngle = -70.0;
                    }
                }
                else if ((angleHelper <= 70.0) || (angleHelper >= 180.0))
                {
                    dashAngle = -70.0;
                }
            }

            if (sidewaysAngle != 0.0)
            {
                dashAngle -= sidewaysAngle;
                if (dashAngle > 180.0)
                {
                    dashAngle -= 360.0;
                }
                if (dashAngle < -180.0)
                {
                    dashAngle += 360.0;
                }
            }
            sidewaysAngle = 1.0;
        }

        float dashRadians = dashAngle * 0.01745329;
        cocos2d::CCPoint p = CCPoint{(float)std::cos(dashRadians), (float)std::sin(dashRadians)};
        cocos2d::CCPoint mult = p * sidewaysAngle;

        float x = mult.x;
        if (player->m_isSideways)
        {
            mult.x = mult.y;
            mult.y = x;
        }

        double doubleX = mult.x;
        player->m_dashX = doubleX;
        player->m_dashY = mult.y;
        if (!player->m_isPlatformer)
        {
            doubleX = mult.y / fabs(mult.x);
            player->m_dashY = doubleX;
            player->m_dashX = fabs(doubleX);
        }
        else if (doubleX >= 0.0)
        {
            if (doubleX > 0.0)
            {
                player->doReversePlayer(false);
            }
        }
        else
        {
            player->doReversePlayer(true);
        }

        player->m_dashAngle = dashAngle;
        player->m_lastGroundedPos = cocos2d::CCPoint{0.0, 0.0};
    }

    void stopDashing(PlayerObject* player)
    {
        if (!player->m_isDashing) { return; }

        player->m_isDashing = false;
        player->m_lastLandTime = 0.0;
        if (player->m_isPlatformer && player->m_dashRing)
        {
            cocos2d::CCPoint boosted = {
                (float)player->m_dashX * player->m_dashRing->m_endBoost,
                (float)player->m_dashY * player->m_dashRing->m_endBoost,
            };
            player->m_isAccelerating = true;
            player->m_yVelocity = boosted.y;

            player->m_platformerXVelocity = boosted.x;
            player->m_affectedByForces = true;

            if (player->m_dashRing->m_stopSlide)
            {
                player->m_isAccelerating = false;
                player->m_affectedByForces = false;
            }
        }
        player->m_dashRing = nullptr;

        if (player->m_isPlatformer)
        {
            float locationNormal = sqrtf(player->m_dashX * player->m_dashX + player->m_dashY * player->m_dashY);

            if (locationNormal <= 17.3) { locationNormal = (locationNormal / 17.3) * 1.5 + 0.5; }
            else { locationNormal = 2.0; }

            float vehicleSizeDependant = player->m_vehicleSize == 1.0 ? 0.433333 : 0.333333;
            float sidewaysMod = player->m_isSideways ? -1 : 1;
            int upsideDownMod = player->m_isUpsideDown ? -0xb4 : 0xb4;
            int leftMod = player->m_isGoingLeft ? -1 : 1;

            player->m_rotationSpeed = (upsideDownMod * leftMod * sidewaysMod * player->m_gravityMod * locationNormal) / (vehicleSizeDependant);
            player->m_isRotating = true;
        }

        if (player->m_isBall)
        {
            if (player->m_isPlatformer)
            {
                player->m_isRotating = false;
                player->m_isBallRotating = false;
                player->m_rotationSpeed = 0.0;
            }

            player->runBallRotation(1.0);
        }
    }

    void ringJump(PlayerObject* player, RingObject* ring)
    {
        if (player->m_isDead) { return; }
        if (!ring) { return; }

        if (player->m_ringRelatedSet.contains(ring->m_uniqueID)) { return; }

        bool isCustomOrTeleportRing =
            (ring->m_objectType == GameObjectType::CustomRing) ||
            (ring->m_objectType == GameObjectType::TeleportOrb);

        if (!player->m_stateRingJump2) { return; }
        if (player->m_isDashing) { return; }
        if (!player->m_stateJumpBuffered) { return; }

        if ((player->m_touchedRing || isCustomOrTeleportRing) &&
            (player->m_touchedCustomRing || ring->m_objectType != GameObjectType::CustomRing))
        {
            if (player->m_touchedGravityPortal) { return; }
            if (ring->m_objectType != GameObjectType::TeleportOrb) { return; }
        }

        if (ring->m_isReverse)
        {
            player->reversePlayer(ring);
        }

        player->m_ringJumpRelated = true;

        player->m_ringRelatedSet.insert(ring->m_uniqueID);

        if (ring->m_objectType == GameObjectType::CustomRing) { player->m_touchedCustomRing = true; }
        else if (ring->m_objectType == GameObjectType::TeleportOrb) { player->m_touchedGravityPortal = true; }
        else { player->m_touchedRing = true; }

        player->m_touchingRings->removeObject(ring, true);
        if (!isCustomOrTeleportRing)
        {
            player->m_padRingRelated = true;
        }

        if (ring->m_objectType == GameObjectType::CustomRing)
        {
            return;
        }

        if (ring->m_objectType == GameObjectType::TeleportOrb)
        {
            teleportPlayer(player->m_gameLayer, (TeleportPortalObject*)ring, player);
            return;
        }

        if (ring->m_objectType == GameObjectType::SpiderOrb)
        {
            if (!player->m_isSideways)
            {
                if (ring->isFacingDown() != player->m_isUpsideDown)
                {
                    flipGravity(player, !player->m_isUpsideDown);
                }
            }
            else
            {
                if (ring->isFacingLeft() != player->m_isUpsideDown)
                {
                    flipGravity(player, !player->m_isUpsideDown);
                }
            }

            player->spiderTestJumpInternal(false);
        }

        if (ring->m_objectType == GameObjectType::DashRing)
        {
            startDashing(player, (DashRingObject*)ring);
            return;
        }

        if (ring->m_objectType == GameObjectType::GravityDashRing)
        {
            flipGravity(player, !player->m_isUpsideDown);
            startDashing(player, (DashRingObject*)ring);
            return;
        }

        player->m_stateRingJump = false;
        if (ring->m_objectType == GameObjectType::DropRing)
        {
            float gravity = player->m_isUpsideDown ? -1.0 : 1.0;
            float gravityMultiplied = gravity * -15.0;

            if (player->m_isShip || player->m_isBird || player->m_isDart || player->m_isSwing)
            {
                gravityMultiplied = gravity * -14.0;
                if (player->m_isBird)
                {
                    gravityMultiplied *= 0.8;
                }
            }
            else if (!player->m_isRobot && player->m_isSpider)
            {
                gravityMultiplied *= 1.1;
            }

            player->setYVelocity(gravityMultiplied, 1);

            if (!player->m_isBall && !player->m_isLocked && !player->m_isDashing)
            {
                player->m_isRotating = false;
                player->m_isBallRotating2 = false;
                player->m_isBallRotating = false;
                player->m_rotationSpeed = 0.0;
                player->runNormalRotation(0, 1.0);
            }
            else
            {
                player->runBallRotation2();
            }
            player->m_hasEverHitRing = true;
            player->m_isAccelerating = true;
            if (player->m_isBall || player->m_isSwing)
            {
                player->m_jumpBuffered = false;
            }

            return;
        }

        player->m_maybeIsBoosted = true;
        player->m_isOnGround2 = false;
        player->m_isOnGround = false;
        float gravity = 1.0;
        if (player->m_vehicleSize != 1.0)
        {
            gravity = 0.8;
        }

        float yStart = player->m_yStart;
        if (ring->m_objectType == GameObjectType::GravityRing)
        {
            yStart *= 0.8;
        }
        else if (ring->m_objectType == GameObjectType::GreenRing)
        {
            if (player->m_isShip)
            {
                yStart *= 0.7;
            }
        }
        else if (ring->m_objectType == GameObjectType::PinkJumpRing)
        {
            if (player->m_isShip) { yStart *= 0.37; }
            else if (player->m_isBird) { yStart *= 0.42; }
            else if (player->m_isBall) { yStart *= 0.77; }
            else { yStart *= 0.72; }
        }
        else if (ring->m_objectType == GameObjectType::RedJumpRing)
        {
            player->m_isAccelerating = true;
            if (!player->m_isShip)
            {
                if (player->m_isBird)
                {
                    if (player->m_vehicleSize == 1.0) { yStart *= 1.02; }
                    else { yStart *= 1.36; }
                }
                else if (player->m_isBall) { yStart *= 1.34; }
                else if (player->m_isRobot) { yStart *= 1.28; }
                else if (player->m_isSpider) { yStart *= 1.34; }
                else { yStart *= 1.38; }
            }
            else if (player->m_vehicleSize != 1.0)
            {
                yStart *= 1.4;
            }
        }
        else if (player->m_isRobot)
        {
            yStart *= 0.9;
        }

        if (ring->m_objectType == GameObjectType::GreenRing)
        {
            flipGravity(player, !player->m_isUpsideDown);
        }

        float gravityCoeff = player->m_isUpsideDown ? -1 : 1;
        double velocity = gravityCoeff * yStart * gravity;
        player->setYVelocity(velocity, gravityCoeff);

        if (!player->m_isBall && !player->m_isLocked && !player->m_isDashing)
        {
            player->m_isRotating = false;
            player->m_isBallRotating2 = false;
            player->m_isBallRotating = false;
            player->m_rotationSpeed = 0.0;
            player->runNormalRotation(0, 1.0);
        }
        else
        {
            player->runBallRotation2();
        }

        cocos2d::CCPoint playerPos = player->getPosition();
        player->m_lastGroundedPos = playerPos;

        player->m_hasEverHitRing = true;
        if (player->m_isSwing)
        {
            player->m_yVelocity *= 0.6;
        }
        else if (player->m_isBall || player->m_isSpider)
        {
            player->m_yVelocity *= 0.7;
        }

        if (ring->m_objectType == GameObjectType::GravityRing)
        {
            flipGravity(player, !player->m_isUpsideDown);
        }

        if (ring->m_objectType == GameObjectType::RedJumpRing)
        {
            player->m_isAccelerating = true;
        }
    }

    void togglePlayerScale(PlayerObject* player, bool smallSize)
    {
        if (player->m_vehicleSize == 1.0 && !smallSize) { return; }
        if (smallSize)
        {
            if (player->m_vehicleSize != 1.0) { return; }

            player->m_vehicleSize = 0.6;
        }
        else
        {
            if (player->m_isPlatformer)
            {
                player->m_stateScale = 2;
            }

            player->m_vehicleSize = 1;
        }

        player->m_spriteWidthScale = player->m_vehicleSize;
        player->m_spriteHeightScale = player->m_vehicleSize;
        player->setScaleX(player->m_vehicleSize);
        player->setScaleY(player->m_vehicleSize);
    }

    static void bumpPlayerFromGJBGL(GJBaseGameLayer* pl, PlayerObject* player, EffectGameObject* object)
    {
        if (!pl->canBeActivatedByPlayer(player, object)) { return; }

        cocos2d::CCPoint objPos = object->getPosition();

        player->m_lastPortalPos = objPos;
        activateForTrajectory(object, player);
        float force = 1.0;

        if (object->m_objectType == GameObjectType::PinkJumpPad)
        {
            if (player->m_isShip) { force = 0.35; }
            else if (player->m_isBird) { force = 0.4; }
            else if (player->m_isBall) { force = 0.7; }
            else if (player->m_isSpider) { force = 0.7; }
            else { force = 0.65; }
        }
        else if (object->m_objectType == GameObjectType::RedJumpPad)
        {
            if (player->m_isShip)
            {
                if (player->m_vehicleSize >= 1.0) { force = 0.63; }
                else { force = 0.95; }
            }
            else if (player->m_isBird)
            {
                if (player->m_vehicleSize >= 1.0) { force = 0.6; }
                else { force = 0.98; }
            }
            else
            {
                force = 1.25;
            }
        }
        player->m_lastActivatedPortal = object;
        bumpPlayer(player, force, static_cast<int>(object->m_objectType), false, object);
    }

    static bool activatingPortal(GJBaseGameLayer* pl, PlayerObject* player, EffectGameObject* portal)
    {
        if (!pl->canBeActivatedByPlayer(player, portal)) { return false; }
        if (TrajectoryManager::get().playerHasActivated(player, portal)) { return false; }

        pl->playerWillSwitchMode(player, portal);

        bool isTheSame =
            ((portal->m_objectType == GameObjectType::ShipPortal) && player->m_isShip) ||
            ((portal->m_objectType == GameObjectType::BallPortal) && player->m_isBall) ||
            ((portal->m_objectType == GameObjectType::UfoPortal) && player->m_isBird) ||
            ((portal->m_objectType == GameObjectType::WavePortal) && player->m_isDart) ||
            ((portal->m_objectType == GameObjectType::SpiderPortal) && player->m_isSpider) ||
            ((portal->m_objectType == GameObjectType::SwingPortal) && player->m_isSwing) ||
            ((portal->m_objectType == GameObjectType::RobotPortal) && player->m_isRobot) ||
            ((portal->m_objectType == GameObjectType::CubePortal) &&
             (!(player->m_isShip || player->m_isBall || player->m_isBird ||
                player->m_isDart || player->m_isSpider || player->m_isSwing)));

        cocos2d::CCPoint position = player->getPosition();
        player->switchedToMode(portal->m_objectType);

        switch (portal->m_objectType)
        {
            case GameObjectType::ShipPortal:
                player->toggleFlyMode(true, true);
                break;
            case GameObjectType::BallPortal:
                player->toggleRollMode(true, true);
                break;
            case GameObjectType::UfoPortal:
                player->toggleBirdMode(true, true);
                break;
            case GameObjectType::WavePortal:
                player->toggleDartMode(true, true);
                break;
            case GameObjectType::SpiderPortal:
                player->toggleSpiderMode(true, true);
                break;
            case GameObjectType::SwingPortal:
                player->toggleSwingMode(true, true);
                break;
            case GameObjectType::RobotPortal:
                player->toggleRobotMode(true, true);
                break;
            default:
                break;
        }

        player->setPosition(position);
        player->m_iconSprite->setPosition(position);
        player->m_vehicleSprite->setPosition(position);

        player->m_lastPortalPos = portal->getPosition();
        player->m_lastActivatedPortal = portal;
        activateForTrajectory(portal, player);

        return !isTheSame;
    }

    void collisionCheckObjects(GJBaseGameLayer* pl, PlayerObject* player, gd::vector<GameObject*>* objects, int objectCount, float dt)
    {
        if (objectCount <= 0) { return; }

        auto& trajectory = TrajectoryManager::get();

        CCRect playerRect = player->getObjectRect();

        for (int i = 0; i < objectCount; i++)
        {
            GameObject* object = objects->at(i);

            if (!object) { continue; }

            if (object->m_objectType == GameObjectType::Decoration ||
                object->m_objectType == GameObjectType::CollisionObject ||
                object->m_objectType == GameObjectType::SecretCoin ||
                object->m_objectType == GameObjectType::UserCoin ||
                object->m_objectType == GameObjectType::Collectible ||
                object->m_objectType == GameObjectType::EnterEffectObject ||
                object->m_objectID == 286 || object->m_objectID == 287 ||
                object->m_isGroupDisabled || object->m_isDisabled)
            {
                continue;
            }

            if (object->m_objectType == GameObjectType::Solid || object->m_objectType == GameObjectType::Breakable)
            {
                if (pl->m_solidCollisionObjectsCount < pl->m_solidCollisionObjectsIndex)
                {
                    pl->m_solidCollisionObjects.at(pl->m_solidCollisionObjectsCount) = object;
                    pl->m_solidCollisionObjectsCount++;
                }
                else
                {
                    pl->m_solidCollisionObjects.push_back(object);
                    pl->m_solidCollisionObjectsCount++;
                    pl->m_solidCollisionObjectsIndex++;
                }

                continue;
            }

            if (object == pl->m_anticheatSpike) { continue; }

            if (object->m_objectType == GameObjectType::Hazard || object->m_objectType == GameObjectType::AnimatedHazard)
            {
                if (pl->m_hazardCollisionObjectsCount < pl->m_hazardCollisionObjectsIndex)
                {
                    pl->m_hazardCollisionObjects.at(pl->m_hazardCollisionObjectsCount) = object;
                    pl->m_hazardCollisionObjectsCount++;
                }
                else
                {
                    pl->m_hazardCollisionObjects.push_back(object);
                    pl->m_hazardCollisionObjectsCount++;
                    pl->m_hazardCollisionObjectsIndex++;
                }

                continue;
            }

            EffectGameObject* obj = (EffectGameObject*)object;

            if ((trajectory.playerHasActivated(player, obj) || trajectory.realPlayerHasActivated(player, obj)) &&
                (object->m_objectType != GameObjectType::Slope))
            {
                continue;
            }

            cocos2d::CCRect rect;
            if (object->m_objectType == GameObjectType::Slope) { rect = object->getObjectRect(2.0, 2.0); }
            else { rect = object->getObjectRect(); }

            if (object->m_objectRadius <= 0.0)
            {
                if (!playerRect.intersectsRect(rect)) { continue; }
            }
            else if (!pl->playerCircleCollision(player, object))
            {
                continue;
            }

            bool overlaps = true;

            if (object->m_shouldUseOuterOb && (!pl->m_levelSettings->m_fixRadiusCollision || object->m_objectRadius <= 0.0))
            {
                OBB2D* box = object->getOrientedBox();
                player->updateOrientedBox();
                OBB2D* playerBox = ((GameObject*)(player))->getOrientedBox();
                overlaps = box->overlaps1Way(playerBox);
            }

            if (!overlaps) { continue; }

            switch (object->m_objectType)
            {
                case GameObjectType::InverseGravityPortal:
                    player->m_lastPortalPos = object->getPosition();
                    player->m_lastActivatedPortal = object;
                    activateForTrajectory(obj, player);
                    flipGravity(player, true);
                    break;
                case GameObjectType::NormalGravityPortal:
                    player->m_lastPortalPos = object->getPosition();
                    player->m_lastActivatedPortal = object;
                    activateForTrajectory(obj, player);
                    flipGravity(player, false);
                    break;
                case GameObjectType::GravityTogglePortal:
                    player->m_lastPortalPos = object->getPosition();
                    player->m_lastActivatedPortal = object;
                    activateForTrajectory(obj, player);
                    flipGravity(player, !player->m_isUpsideDown);
                    break;
                case GameObjectType::TeleportPortal:
                    if (pl->canBeActivatedByPlayer(player, obj))
                    {
                        teleportPlayer(pl, (TeleportPortalObject*)object, player);
                        activateForTrajectory(obj, player);
                    }
                    break;
                case GameObjectType::Slope:
                    if (!player->m_isSideways)
                    {
                        player->collidedWithSlopeInternal(dt, object, false);
                    }
                    else
                    {
                        cocos2d::CCRect emptyRect = cocos2d::CCRect{0.0, 0.0, 0.0, 0.0};
                        player->handleRotatedCollisionInternal(dt, object, emptyRect, false, false, true);
                    }
                    break;
                case GameObjectType::CustomRing:
                case GameObjectType::DashRing:
                case GameObjectType::DropRing:
                case GameObjectType::GravityDashRing:
                case GameObjectType::GravityRing:
                case GameObjectType::GreenRing:
                case GameObjectType::PinkJumpRing:
                case GameObjectType::RedJumpRing:
                case GameObjectType::SpiderOrb:
                case GameObjectType::YellowJumpRing:
                case GameObjectType::TeleportOrb:
                    if (!player->m_touchingRings->containsObject(object))
                    {
                        player->m_touchingRings->addObject(object);
                    }
                    player->m_touchedRings.insert(object->m_uniqueID);

                    if (!player->m_isShip && !player->m_isBird && !player->m_isDart && !player->m_isSwing &&
                        !((RingObject*)object)->m_isSpawnOnly)
                    {
                        ringJump(player, (RingObject*)object);
                        activateForTrajectory(obj, player);
                    }
                    break;
                case GameObjectType::YellowJumpPad:
                case GameObjectType::PinkJumpPad:
                case GameObjectType::RedJumpPad:
                case GameObjectType::SpiderPad:
                    bumpPlayerFromGJBGL(pl, player, obj);
                    break;
                case GameObjectType::GravityPad:
                {
                    bool isFacingDown = false;
                    if (player->m_isSideways) { isFacingDown = object->isFacingLeft(); }
                    else { isFacingDown = object->isFacingDown(); }

                    bool canBeActivated = pl->canBeActivatedByPlayer(player, obj);
                    if (player->m_isUpsideDown == isFacingDown && canBeActivated)
                    {
                        if (obj->m_isReverse)
                        {
                            player->reversePlayer(obj);
                        }

                        player->m_lastPortalPos = obj->getPosition();
                        player->m_lastActivatedPortal = obj;
                        activateForTrajectory(obj, player);

                        propellPlayer(player, 0.8, false, 10);
                        flipGravity(player, !isFacingDown);
                        player->m_padRingRelated = true;
                    }
                    break;
                }
                case GameObjectType::MiniSizePortal:
                    if (pl->canBeActivatedByPlayer(player, obj))
                    {
                        player->m_lastPortalPos = obj->getPosition();
                        player->m_lastActivatedPortal = obj;
                        activateForTrajectory(obj, player);

                        togglePlayerScale(player, true);
                    }
                    break;
                case GameObjectType::RegularSizePortal:
                    if (pl->canBeActivatedByPlayer(player, obj))
                    {
                        player->m_lastPortalPos = obj->getPosition();
                        player->m_lastActivatedPortal = obj;
                        activateForTrajectory(obj, player);

                        togglePlayerScale(player, false);
                    }
                    break;
                case GameObjectType::Special:
                    if (object->m_objectID == 0x743)
                    {
                        player->m_stateHitHead = 2;
                    }
                    else if (object->m_objectID == 0x6db)
                    {
                        player->m_stateDartSlide = 2;
                    }
                    else if (object->m_objectID == 0x715)
                    {
                        player->m_stateNoAutoJump = 2;
                    }
                    else if (object->m_objectID == 0x725 && player->m_isDashing)
                    {
                        stopDashing(player);
                        player->m_jumpBuffered = false;
                    }
                    else if (object->m_objectID == 0xb32)
                    {
                        player->m_stateFlipGravity = 2;
                    }
                    else if (object->m_objectID == 2069 || object->m_objectID == 3645)
                    {
                        player->m_stateForce = 2;
                        ForceBlockGameObject* forceBlock = (ForceBlockGameObject*)object;
                        int forceID = forceBlock->m_forceID;
                        if (forceID > 0)
                        {
                            if (player->m_jumpPadRelated.contains(forceID))
                            {
                                if (player->m_jumpPadRelated.at(forceID))
                                {
                                    break;
                                }
                            }
                            player->m_jumpPadRelated.insert({forceID, true});
                        }

                        CCPoint force = forceBlock->calculateForceToTarget(player);
                        player->m_stateForceVector += force;
                    }
                    break;
                case GameObjectType::CubePortal:
                case GameObjectType::ShipPortal:
                case GameObjectType::BallPortal:
                case GameObjectType::UfoPortal:
                case GameObjectType::WavePortal:
                case GameObjectType::SpiderPortal:
                case GameObjectType::SwingPortal:
                case GameObjectType::RobotPortal:
                    activatingPortal(pl, player, obj);
                    break;
                case GameObjectType::Modifier:
                    obj->activatedByPlayer(player);

                    if (obj->m_isTouchTriggered)
                    {
                        triggerObject(obj, pl, player);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void triggerObject(EffectGameObject* object, GJBaseGameLayer* pl, PlayerObject* player)
    {
        auto& trajectory = TrajectoryManager::get();

        switch (object->m_objectID)
        {
            case 200:
                *(float*)(&pl->m_gameState.m_timeModRelated) = 0.7;
                break;
            case 201:
                *(float*)(&pl->m_gameState.m_timeModRelated) = 0.9;
                break;
            case 202:
                *(float*)(&pl->m_gameState.m_timeModRelated) = 1.1;
                break;
            case 203:
                *(float*)(&pl->m_gameState.m_timeModRelated) = 1.3;
                break;
            case 1334:
                *(float*)(&pl->m_gameState.m_timeModRelated) = 1.6;
                break;
            case 2066:
            {
                if (object->m_followCPP) { break; }

                bool isP2 = trajectory.isFakePlayer2(player);
                if (!object->m_targetPlayer2 && !isP2)
                {
                    player->m_gravityMod = object->m_gravityValue;
                }
                if (object->m_targetPlayer2 && isP2)
                {
                    player->m_gravityMod = object->m_gravityValue;
                }
                break;
            }
            case 2900:
            {
                RotateGameplayGameObject* rotateObj = (RotateGameplayGameObject*)object;

                if (rotateObj->m_changeChannel)
                {
                    pl->m_gameState.m_currentChannel = rotateObj->m_targetChannelID;
                    pl->m_gameState.m_spawnChannelRelated1[rotateObj->m_targetChannelID] =
                        rotateObj->m_groundDirection == 2 || rotateObj->m_groundDirection == 3;
                }

                if (rotateObj->m_channelOnly) { break; }

                auto rotate = [rotateObj](PlayerObject* p)
                {
                    p->rotateGameplay(
                        rotateObj->m_moveDirection, rotateObj->m_groundDirection,
                        rotateObj->m_editVelocity, rotateObj->m_velocityModX,
                        rotateObj->m_velocityModY, rotateObj->m_overrideVelocity,
                        rotateObj->m_dontSlide);
                };

                rotate(player);

                if (pl->m_gameState.m_isDualMode && trajectory.isFakePlayer(player))
                {
                    PlayerObject* other = trajectory.getOtherPlayer(player);
                    if (other && other != player &&
                        !trajectory.playerHasActivated(other, object) &&
                        !trajectory.realPlayerHasActivated(other, object))
                    {
                        rotate(other);
                        activateForTrajectory(object, other);
                    }
                }

                break;
            }
            case 3022:
            {
                teleportPlayer(pl, (TeleportPortalObject*)object, player);
                break;
            }
            default:
                break;
        }

        activateForTrajectory(object, player);
    }

    void checkSpawnObjects(GJBaseGameLayer* pl, PlayerObject* player)
    {
        CCPoint position = player->getPosition();

        if (!pl->m_spawnObjects) { return; }

        cocos2d::CCArray* objects = (cocos2d::CCArray*)pl->m_spawnObjects->objectForKey(pl->m_gameState.m_currentChannel);
        if (!objects) { return; }

        const int channel = pl->m_gameState.m_currentChannel;
        const auto& related0 = pl->m_gameState.m_spawnChannelRelated0;
        const auto& related1 = pl->m_gameState.m_spawnChannelRelated1;

        auto startIt = related0.find(channel);
        auto backIt = related1.find(channel);

        int startingIndex = startIt == related0.end() ? 0 : startIt->second;
        bool goingBack = backIt == related1.end() ? false : backIt->second;

        auto& trajectory = TrajectoryManager::get();

        for (int i = startingIndex; static_cast<unsigned int>(i) < objects->count(); i++)
        {
            SpawnTriggerGameObject* object = (SpawnTriggerGameObject*)objects->objectAtIndex(i);
            if (object->m_objectID != 2066 && object->m_objectID != 2900 &&
                object->m_objectID != 3022 && object->m_objectID != 901)
            {
                continue;
            }

            CCPoint objectPos = object->m_speedStart;

            if (player->m_isSideways)
            {
                if (goingBack)
                {
                    if (objectPos.y < position.y) { break; }
                }
                else
                {
                    if (objectPos.y > position.y) { break; }
                }
            }
            else
            {
                if (goingBack)
                {
                    if (objectPos.x < position.x) { break; }
                }
                else
                {
                    if (objectPos.x > position.x) { break; }
                }
            }

            if (object->m_isGroupDisabled) { continue; }
            if (trajectory.playerHasActivated(player, object) || trajectory.realPlayerHasActivated(player, object))
            {
                continue;
            }

            if (!object->m_isTouchTriggered)
            {
                triggerObject(object, pl, player);
            }
        }
    }
}
