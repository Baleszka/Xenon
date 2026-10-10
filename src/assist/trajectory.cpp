#include <Geode/Geode.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <functional>
#include <vector>

#include "trajectory.hpp"
#include "trajectoryPhysics.hpp"
#include "hitboxes.hpp"
#include "autoclicker.hpp"
#include "../bot/replay.hpp"
#include "../bot/cbf.hpp"
#include "../bot/frameHook.hpp"
#include "../bot/tpsBypass.hpp"

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/EnhancedGameObject.hpp>
#include <Geode/modify/CCScheduler.hpp>

using namespace geode::prelude;

namespace
{
    struct SimInput
    {
        unsigned long long frame;
        bool isHolding;
        bool isPlayer1;
    };

    struct AutoclickerSim
    {
        bool enabled;
        int counter;
        int holdFor;
        int releaseFor;
        bool swift;
        bool isPlayer1;
    };

    cocos2d::ccColor4F toColor(const float* c)
    {
        return cocos2d::ccColor4F{c[0], c[1], c[2], c[3]};
    }

    cocos2d::ccColor4F toColorNegative(const float* c)
    {
        return cocos2d::ccColor4F{1.0f - c[0], 1.0f - c[1], 1.0f - c[2], c[3]};
    }

    uint64_t packPlayerFlags(PlayerObject* p)
    {
        uint64_t flags = 0;
        int bit = 0;
        auto set = [&](bool v)
        {
            if (v) { flags |= (uint64_t(1) << bit); }
            bit++;
        };
        set(p->m_isShip);
        set(p->m_isBird);
        set(p->m_isDart);
        set(p->m_isSwing);
        set(p->m_isBall);
        set(p->m_isSpider);
        set(p->m_isRobot);
        set(p->m_isUpsideDown);
        set(p->m_isDead);
        set(p->m_isOnGround2);
        set(p->m_isSideways);
        set(p->m_isDashing);
        set(p->m_isAccelerating);
        set(p->m_holdingLeft);
        set(p->m_holdingRight);
        set(p->m_jumpBuffered);
        set(p->m_isPlatformer);
        set(p->m_isGoingLeft);
        set(p->m_maybeIsBoosted);
        return flags;
    }

    size_t hashColors()
    {
        size_t h = 0;
        const float* colors[] = { holdTrajectoryCol, releaseTrajectoryCol, swiftTrajectoryCol, hazardCol, hazardRotCol, solidCol };
        for (const float* color : colors)
        {
            for (int i = 0; i < 4; i++)
            {
                h = h * 31 + std::hash<float>{}(color[i]);
            }
        }
        return h;
    }

#define XE_GS_CONTAINERS(X) \
    X(m_spawnChannelRelated0) X(m_spawnChannelRelated1) X(m_tweenActions) X(m_gameObjectPhysics) \
    X(m_unkVecFloat1) X(m_activatedObjectIDs) X(m_stateObjects) \
    X(m_unkMapPairGJGameEventIntVectorEventTriggerInstance) X(m_unkMapPairGJGameEventIntInt) \
    X(m_enterEffectInstanceVectors) X(m_exitEffectInstanceVectors) X(m_enterChannelMap) X(m_exitChannelMap) \
    X(m_moveEffectInstances) X(m_rotateEffectInstances) X(m_scaleEffectInstances) X(m_fadeEffectInstances) \
    X(m_tintEffectInstances) X(m_unsortedAreaEffects) X(m_advanceFollowInstances) X(m_dynamicMoveActions) \
    X(m_dynamicRotateActions) X(m_unkUMap8) X(m_proximityVolumeRelated) X(m_songChannelStates) \
    X(m_songTriggerStateVectors) X(m_sfxTriggerStates)

    struct GameStateSnapshot
    {
        std::array<unsigned char, sizeof(GJGameState)> raw;

#define XE_GS_DECLARE(member) decltype(GJGameState::member) member;
        XE_GS_CONTAINERS(XE_GS_DECLARE)
#undef XE_GS_DECLARE

        void save(const GJGameState& state)
        {
            std::memcpy(raw.data(), static_cast<const void*>(&state), sizeof(GJGameState));

#define XE_GS_SAVE(member) member = state.member;
            XE_GS_CONTAINERS(XE_GS_SAVE)
#undef XE_GS_SAVE
        }

        void restore(GJGameState& state)
        {
#define XE_GS_RESTORE(member) state.member = member;
            XE_GS_CONTAINERS(XE_GS_RESTORE)
#undef XE_GS_RESTORE

            std::array<unsigned char, sizeof(GJGameState)> merged = raw;
            const unsigned char* base = reinterpret_cast<const unsigned char*>(&state);

#define XE_GS_KEEP(member) std::memcpy(merged.data() + (reinterpret_cast<const unsigned char*>(&state.member) - base), static_cast<const void*>(&state.member), sizeof(state.member));
            XE_GS_CONTAINERS(XE_GS_KEEP)
#undef XE_GS_KEEP

            std::memcpy(static_cast<void*>(&state), merged.data(), sizeof(GJGameState));
        }
    };

#undef XE_GS_CONTAINERS

    GameStateSnapshot gameStateSnapshot;

    void copyPlayerState(PlayerObject* from, PlayerObject* to)
    {
#define XE_COPY(field) to->field = from->field;
        to->copyAttributes(from);
        to->setPosition(from->m_position);
        to->setRotation(from->getRotation());

        XE_COPY(m_wasTeleported) XE_COPY(m_fixGravityBug) XE_COPY(m_reverseSync) XE_COPY(m_yVelocityBeforeSlope)
        XE_COPY(m_dashX) XE_COPY(m_dashY) XE_COPY(m_dashAngle) XE_COPY(m_dashStartTime) XE_COPY(m_dashRing)
        XE_COPY(m_slopeStartTime) XE_COPY(m_justPlacedStreak) XE_COPY(m_maybeLastGroundObject)
        XE_COPY(m_lastCollisionBottom) XE_COPY(m_lastCollisionTop) XE_COPY(m_lastCollisionLeft) XE_COPY(m_lastCollisionRight)
        XE_COPY(m_unk50C) XE_COPY(m_unk510) XE_COPY(m_currentSlope2) XE_COPY(m_preLastGroundObject)
        XE_COPY(m_slopeAngle) XE_COPY(m_slopeSlidingMaybeRotated) XE_COPY(m_quickCheckpointMode)
        XE_COPY(m_collidedObject) XE_COPY(m_lastGroundObject) XE_COPY(m_collidingWithLeft) XE_COPY(m_collidingWithRight)
        XE_COPY(m_maybeSavedPlayerFrame) XE_COPY(m_scaleXRelated2) XE_COPY(m_groundYVelocity) XE_COPY(m_yVelocityRelated)
        XE_COPY(m_scaleXRelated3) XE_COPY(m_scaleXRelated4) XE_COPY(m_scaleXRelated5)
        XE_COPY(m_isCollidingWithSlope) XE_COPY(m_isBallRotating) XE_COPY(m_unk669)
        XE_COPY(m_currentPotentialSlope) XE_COPY(m_currentSlope) XE_COPY(unk_584) XE_COPY(m_collidingWithSlopeId)
        XE_COPY(m_slopeFlipGravityRelated) XE_COPY(m_slopeAngleRadians) XE_COPY(m_rotateObjectsRelated) XE_COPY(m_potentialSlopeMap)
        XE_COPY(m_rotationSpeed) XE_COPY(m_rotateSpeed) XE_COPY(m_isRotating) XE_COPY(m_isBallRotating2)
        XE_COPY(m_hasGlow) XE_COPY(m_isHidden) XE_COPY(m_speedMultiplier) XE_COPY(m_yStart) XE_COPY(m_gravity)
        XE_COPY(m_trailingParticleLife) XE_COPY(m_unk648) XE_COPY(m_gameModeChangedTime) XE_COPY(m_padRingRelated)
        XE_COPY(m_maybeIsFalling) XE_COPY(m_maybeCanRunIntoBlocks) XE_COPY(m_hasGroundParticles) XE_COPY(m_hasShipParticles)
        XE_COPY(m_isOnGround3) XE_COPY(m_checkpointTimeout) XE_COPY(m_lastCheckpointTime) XE_COPY(m_lastJumpTime)
        XE_COPY(m_lastFlipTime) XE_COPY(m_flashTime) XE_COPY(m_flashDuration) XE_COPY(m_flashDelay)
        XE_COPY(m_lastSpiderFlipTime) XE_COPY(m_unkBool5) XE_COPY(m_maybeIsVehicleGlowing)
        XE_COPY(m_switchWaveTrailColor) XE_COPY(m_practiceDeathEffect) XE_COPY(m_accelerationOrSpeed) XE_COPY(m_snapDistance)
        XE_COPY(m_ringJumpRelated) XE_COPY(m_ringRelatedSet) XE_COPY(m_objectSnappedTo) XE_COPY(m_onFlyCheckpointTries)
        XE_COPY(m_maybeSpriteRelated) XE_COPY(m_useLandParticles0) XE_COPY(m_landParticlesAngle) XE_COPY(m_landParticleRelatedY)
        XE_COPY(m_playerStreak) XE_COPY(m_streakStrokeWidth) XE_COPY(m_disableStreakTint) XE_COPY(m_alwaysShowStreak)
        XE_COPY(m_shipStreakType) XE_COPY(m_slopeRotation) XE_COPY(m_currentSlopeYVelocity) XE_COPY(m_unk3d0)
        XE_COPY(m_blackOrbRelated) XE_COPY(m_unk3e0) XE_COPY(m_unk3e1) XE_COPY(m_isAccelerating) XE_COPY(m_isCurrentSlopeTop)
        XE_COPY(m_collidedTopMinY) XE_COPY(m_collidedBottomMaxY) XE_COPY(m_collidedLeftMaxX) XE_COPY(m_collidedRightMinX)
        XE_COPY(m_fadeOutStreak) XE_COPY(m_canPlaceCheckpoint) XE_COPY(m_hasCustomGlowColor) XE_COPY(m_maybeIsColliding)
        XE_COPY(m_jumpBuffered) XE_COPY(m_stateRingJump) XE_COPY(m_wasJumpBuffered) XE_COPY(m_wasRobotJump)
        XE_COPY(m_stateJumpBuffered) XE_COPY(m_stateRingJump2) XE_COPY(m_touchedRing) XE_COPY(m_touchedCustomRing)
        XE_COPY(m_touchedGravityPortal) XE_COPY(m_maybeTouchedBreakableBlock) XE_COPY(m_jumpRelatedAC2) XE_COPY(m_touchedPad)
        XE_COPY(m_yVelocity) XE_COPY(m_fallSpeed) XE_COPY(m_isOnSlope) XE_COPY(m_wasOnSlope) XE_COPY(m_slopeVelocity)
        XE_COPY(m_maybeUpsideDownSlope) XE_COPY(m_isShip) XE_COPY(m_isBird) XE_COPY(m_isBall) XE_COPY(m_isDart)
        XE_COPY(m_isRobot) XE_COPY(m_isSpider) XE_COPY(m_isUpsideDown) XE_COPY(m_isDead) XE_COPY(m_isOnGround)
        XE_COPY(m_isGoingLeft) XE_COPY(m_isSideways) XE_COPY(m_isSwing) XE_COPY(m_reverseRelated)
        XE_COPY(m_maybeReverseSpeed) XE_COPY(m_maybeReverseAcceleration) XE_COPY(m_xVelocityRelated2) XE_COPY(m_isDashing)
        XE_COPY(m_dashFireFrame) XE_COPY(m_groundObjectMaterial) XE_COPY(m_vehicleSize) XE_COPY(m_playerSpeed)
        XE_COPY(m_shipRotation) XE_COPY(m_lastPortalPos) XE_COPY(m_unkUnused3) XE_COPY(m_isOnGround2) XE_COPY(m_lastLandTime)
        XE_COPY(m_platformerVelocityRelated) XE_COPY(m_maybeIsBoosted) XE_COPY(m_scaleXRelatedTime) XE_COPY(m_decreaseBoostSlide)
        XE_COPY(m_unkA29) XE_COPY(m_isLocked) XE_COPY(m_controlsDisabled) XE_COPY(m_lastGroundedPos)
        XE_COPY(m_touchedRings) XE_COPY(m_lastActivatedPortal) XE_COPY(m_hasEverJumped) XE_COPY(m_hasEverHitRing)
        XE_COPY(m_position) XE_COPY(m_isSecondPlayer) XE_COPY(m_unkA99) XE_COPY(m_totalTime)
        XE_COPY(m_isBeingSpawnedByDualPortal) XE_COPY(m_audioScale) XE_COPY(m_unkAngle1) XE_COPY(m_yVelocityRelated3)
        XE_COPY(m_defaultMiniIcon) XE_COPY(m_swapColors) XE_COPY(m_switchDashFireColor) XE_COPY(m_followRelated)
        XE_COPY(m_playerFollowFloats) XE_COPY(m_unk838) XE_COPY(m_stateOnGround) XE_COPY(m_stateUnk)
        XE_COPY(m_stateNoStickX) XE_COPY(m_stateNoStickY) XE_COPY(m_stateUnk2) XE_COPY(m_stateBoostX) XE_COPY(m_stateBoostY)
        XE_COPY(m_maybeStateForce2) XE_COPY(m_stateScale) XE_COPY(m_platformerXVelocity) XE_COPY(m_holdingRight)
        XE_COPY(m_holdingLeft) XE_COPY(m_leftPressedFirst) XE_COPY(m_scaleXRelated) XE_COPY(m_maybeHasStopped)
        XE_COPY(m_xVelocityRelated) XE_COPY(m_maybeGoingCorrectSlopeDirection) XE_COPY(m_isSliding) XE_COPY(m_maybeSlopeForce)
        XE_COPY(m_isOnIce) XE_COPY(m_physDeltaRelated) XE_COPY(m_isOnGround4) XE_COPY(m_maybeSlidingTime)
        XE_COPY(m_maybeSlidingStartTime) XE_COPY(m_changedDirectionsTime) XE_COPY(m_slopeEndTime) XE_COPY(m_isMoving)
        XE_COPY(m_platformerMovingLeft) XE_COPY(m_platformerMovingRight) XE_COPY(m_isSlidingRight)
        XE_COPY(m_maybeChangedDirectionAngle) XE_COPY(m_unkUnused2) XE_COPY(m_isPlatformer) XE_COPY(m_stateNoAutoJump)
        XE_COPY(m_stateDartSlide) XE_COPY(m_stateHitHead) XE_COPY(m_stateFlipGravity) XE_COPY(m_gravityMod)
        XE_COPY(m_stateForce) XE_COPY(m_stateForceVector) XE_COPY(m_affectedByForces) XE_COPY(m_jumpPadRelated)
        XE_COPY(m_somethingPlayerSpeedTime) XE_COPY(m_playerSpeedAC) XE_COPY(m_fixRobotJump) XE_COPY(m_holdingButtons)
        XE_COPY(m_inputsLocked) XE_COPY(m_gv0123) XE_COPY(m_iconRequestID) XE_COPY(m_unkUnused) XE_COPY(m_isOutOfBounds)
        XE_COPY(m_fallStartY) XE_COPY(m_disablePlayerSqueeze) XE_COPY(m_robotAnimation1Enabled)
        XE_COPY(m_robotAnimation2Enabled) XE_COPY(m_spiderAnimationEnabled) XE_COPY(m_ignoreDamage) XE_COPY(m_enable22Changes)
#undef XE_COPY

        to->m_touchingRings->removeAllObjects();
        for (unsigned int i = 0; i < from->m_touchingRings->count(); i++)
        {
            to->m_touchingRings->addObject(from->m_touchingRings->objectAtIndex(i));
        }

        to->m_shouldTryPlacingCheckpoint = false;
        to->m_maybeReducedEffects = true;
        to->m_playEffects = false;
    }
}

PlayerObject* Trajectory::createFakePlayer(GJBaseGameLayer* pl, const std::string& id)
{
    PlayerObject* player = PlayerObject::create(1, 1, pl, pl, true);
    player->retain();
    player->setPosition({0, 105});
    player->setID(id);
    player->setVisible(false);
    pl->m_objectLayer->addChild(player);

    return player;
}

Trajectory* Trajectory::create(GJBaseGameLayer* pl)
{
    Trajectory* t = new Trajectory();

    t->m_node = TrajectoryDrawNode::create();
    t->m_node->retain();
    t->m_node->setID("trajectory-node"_spr);
    t->m_node->setBlendFunc(cocos2d::ccBlendFunc{770, 771});

    t->m_fakePlayer1 = createFakePlayer(pl, "trajectory-fake-player1"_spr);
    t->m_fakePlayer2 = createFakePlayer(pl, "trajectory-fake-player2"_spr);

    cocos2d::CCNode* parent = pl->m_debugDrawNode ? pl->m_debugDrawNode->getParent() : nullptr;
    if (!parent) { parent = pl->m_objectLayer; }

    int zOrder = pl->m_uiLayer ? pl->m_uiLayer->getZOrder() + 10000 : 10000;
    parent->addChild(t->m_node, zOrder);

    return t;
}

Trajectory::~Trajectory()
{
    if (m_fakePlayer1) { m_fakePlayer1->m_maybeLastGroundObject = nullptr; }
    if (m_fakePlayer2) { m_fakePlayer2->m_maybeLastGroundObject = nullptr; }

    CC_SAFE_RELEASE(m_fakePlayer1);
    CC_SAFE_RELEASE(m_fakePlayer2);
    CC_SAFE_RELEASE(m_node);
}

bool Trajectory::playerHasActivated(PlayerObject* player, EnhancedGameObject* object)
{
    if (!object) { return false; }
    if (object->m_isMultiActivate) { return false; }

    if (player == m_fakePlayer1) { return m_activatedObjectsP1.contains((uintptr_t)object); }
    if (player == m_fakePlayer2) { return m_activatedObjectsP2.contains((uintptr_t)object); }

    return false;
}

bool Trajectory::realPlayerHasActivated(PlayerObject* player, EnhancedGameObject* object)
{
    if (!object) { return false; }

    if (!isFakePlayer(player)) { return phys::hasBeenActivatedByPlayer(player, object); }

    auto pl = GJBaseGameLayer::get();
    if (!pl) { return false; }

    PlayerObject* realPlayer = player == m_fakePlayer1 ? pl->m_player1 : pl->m_player2;
    if (!realPlayer) { return false; }

    return phys::hasBeenActivatedByPlayer(realPlayer, object);
}

Trajectory::Signature Trajectory::computeSignature(GJBaseGameLayer* pl)
{
    Signature s;
    s.frame = g_frame;

    auto fill = [](PlayerObject* p, float* out, uint64_t& flags)
    {
        const cocos2d::CCPoint pos = p->getPosition();
        out[0] = pos.x;
        out[1] = pos.y;
        out[2] = p->getRotation();
        out[3] = p->m_yVelocity;
        out[4] = p->m_platformerXVelocity;
        out[5] = p->m_gravityMod;
        out[6] = p->m_vehicleSize;
        flags = packPlayerFlags(p);
    };

    if (pl->m_player1) { fill(pl->m_player1, s.p1, s.p1flags); }
    if (pl->m_player2) { fill(pl->m_player2, s.p2, s.p2flags); }

    s.timeWarp = pl->m_gameState.m_timeWarp;
    s.cameraZoom = pl->m_gameState.m_cameraZoom;
    s.width = trajectoryTrailWidth;
    s.hitboxWidth = hitboxWidth;
    s.length = trajectoryLength;
    s.tps = tps;
    s.status = replayStatus;

    s.autoclicker[0] = counterP1;
    s.autoclicker[1] = counterP2;
    s.autoclicker[2] = p1HoldFor;
    s.autoclicker[3] = p1ReleaseFor;
    s.autoclicker[4] = p2HoldFor;
    s.autoclicker[5] = p2ReleaseFor;

    uint32_t b = 0;
    int bit = 0;
    auto setb = [&](bool v)
    {
        if (v) { b |= (uint32_t(1) << bit); }
        bit++;
    };
    setb(showTrajectory);
    setb(holdTrajectory);
    setb(releaseTrajectory);
    setb(swiftTrajectory);
    setb(pl->m_gameState.m_isDualMode);
    setb(pl->m_isPlatformer);
    setb(pl->m_levelSettings->m_twoPlayerMode);
    setb(pl->m_player1 != nullptr);
    setb(pl->m_player2 != nullptr);
    setb(cbf);
    setb(::player1);
    setb(::player2);
    setb(p1Swift);
    setb(p2Swift);
    s.boolPack = b;

    s.colorHash = hashColors();
    s.macroSize = macro.size() + cbfMacro.size() * 31;

    return s;
}

void Trajectory::drawHitbox(PlayerObject* player)
{
    auto pl = GJBaseGameLayer::get();
    float zoom = pl ? pl->m_gameState.m_cameraZoom : 1.0f;
    if (zoom == 0.0f) { zoom = 1.0f; }

    float width = hitboxWidth / zoom;
    cocos2d::CCRect rect = player->getObjectRect();
    cocos2d::CCRect inner = player->getObjectRect(0.3, 0.3);

    std::array<cocos2d::CCPoint, 4> vertices = {
        cocos2d::CCPoint{rect.getMinX(), rect.getMinY()},
        cocos2d::CCPoint{rect.getMaxX(), rect.getMinY()},
        cocos2d::CCPoint{rect.getMaxX(), rect.getMaxY()},
        cocos2d::CCPoint{rect.getMinX(), rect.getMaxY()}};

    for (auto& vertex : vertices)
    {
        vertex = vertex.rotateByAngle({rect.getMidX(), rect.getMidY()}, -CC_DEGREES_TO_RADIANS(player->getRotation()));
    }

    m_node->drawPolygon(vertices.data(), 4, {0.0f, 0.0f, 0.0f, 0.0f}, width, toColor(hazardRotCol), cocos2d::BorderAlignment::Inside);
    m_node->drawRect(rect, {0.0f, 0.0f, 0.0f, 0.0f}, width, toColor(hazardCol), cocos2d::BorderAlignment::Inside);
    m_node->drawRect(inner, {0.0f, 0.0f, 0.0f, 0.0f}, width, toColor(solidCol), cocos2d::BorderAlignment::Inside);
}

bool Trajectory::iterate(GJBaseGameLayer* pl, PlayerObject* player, int mode, float* colors, int& stepCount)
{
    cocos2d::CCPoint position = player->getPosition();

    const float physicsDt = 1.0f / static_cast<float>(std::max(tps, 1));
    float timeWarp = pl->m_gameState.m_timeWarp;
    if (timeWarp == 0.0f) { timeWarp = 1.0f; }

    pl->m_gameState.m_totalTime += physicsDt;
    pl->m_gameState.m_unkDouble3 += physicsDt / timeWarp;
    pl->m_gameState.m_currentProgress++;
    pl->m_gameState.m_unkUint5 += (int)roundf(timeWarp * 1000.0f);

    player->m_totalTime += physicsDt;

    float playerSpeed = *(float*)(&pl->m_gameState.m_timeModRelated);
    if (playerSpeed != 0.0)
    {
        pl->m_gameState.m_timeModRelated = 0;
        pl->m_gameState.m_timeModRelated2 = 0;

        player->updateTimeMod(playerSpeed, true);
    }

    player->m_collisionLogTop->removeAllObjects();
    player->m_collisionLogBottom->removeAllObjects();
    player->m_collisionLogLeft->removeAllObjects();
    player->m_collisionLogRight->removeAllObjects();

    if ((m_deadP1 && (mode & TrajectoryMode::Player1) != 0) || (m_deadP2 && (mode & TrajectoryMode::Player2) != 0))
    {
        if (stepCount > 1) { drawHitbox(player); }
        return true;
    }

    player->m_playEffects = false;
    player->update(m_delta);

    player->m_unkUnused3 = player->getRotation();
    player->updateRotation(m_delta);
    player->m_shipRotation = player->getPosition();

    if (pl->checkCollisions(player, m_delta, false) == 1)
    {
        this->hasDied(player);
    }

    phys::checkSpawnObjects(pl, player);

    float zoom = pl->m_gameState.m_cameraZoom;
    if (zoom == 0.0f) { zoom = 1.0f; }

    m_node->drawSegment(position, player->getPosition(), trajectoryTrailWidth / zoom,
                        player == m_fakePlayer1 ? toColor(colors) : toColorNegative(colors));

    stepCount++;

    return false;
}

void Trajectory::runPrediction(GJBaseGameLayer* pl, PlayerObject* player, PlayerObject* other, int mode, float* colors, bool both)
{
    const int iterations = std::max(trajectoryLength, 0);
    const bool dual = pl->m_gameState.m_isDualMode;
    const bool twoPlayer = pl->m_levelSettings->m_twoPlayerMode;
    const bool recording = replayStatus == 0;
    const bool playing = replayStatus == 1;

    const int playerMask = player == m_fakePlayer1 ? TrajectoryMode::Player1 : TrajectoryMode::Player2;
    const int otherMask = other == m_fakePlayer1 ? TrajectoryMode::Player1 : TrajectoryMode::Player2;

    std::array<PlayerObject*, 2> players = {player, other};
    const int count = (both && dual) ? 2 : 1;

    auto press = [&](bool down, bool isPlayer1)
    {
        for (int k = 0; k < count; k++)
        {
            PlayerObject* target = players[k];
            if (twoPlayer && (target == m_fakePlayer1) != isPlayer1) { continue; }

            if (down) { target->pushButton(PlayerButton::Jump); }
            else { target->releaseButton(PlayerButton::Jump); }
        }
    };

    std::array<AutoclickerSim, 2> autoclickers = {{
        {recording && ::player1, counterP1, p1HoldFor, p1ReleaseFor, p1Swift, true},
        {recording && ::player2, counterP2, p2HoldFor, p2ReleaseFor, p2Swift, false},
    }};

    auto autoclickerDrives = [&](PlayerObject* target)
    {
        for (auto& ac : autoclickers)
        {
            if (!ac.enabled) { continue; }
            if (!twoPlayer || (target == m_fakePlayer1) == ac.isPlayer1) { return true; }
        }
        return false;
    };

    if (recording)
    {
        for (int k = 0; k < count; k++)
        {
            PlayerObject* target = players[k];
            if (autoclickerDrives(target)) { continue; }

            switch (mode & TrajectoryMode::ClickMask)
            {
                case TrajectoryMode::Hold:
                    target->pushButton(PlayerButton::Jump);
                    break;
                case TrajectoryMode::Swift:
                    target->pushButton(PlayerButton::Jump);
                    target->releaseButton(PlayerButton::Jump);
                    break;
                case TrajectoryMode::Release:
                    target->releaseButton(PlayerButton::Jump);
                    target->m_jumpBuffered = false;
                    break;
                default:
                    break;
            }
        }
    }

    std::vector<SimInput> pending;
    if (playing && iterations > 0)
    {
        const unsigned long long first = g_frame;
        const unsigned long long last = g_frame + static_cast<unsigned long long>(iterations);

        if (cbf)
        {
            for (auto& in : cbfMacro)
            {
                if (in.preciseFrame < 0.0) { continue; }
                auto frame = static_cast<unsigned long long>(in.preciseFrame);
                if (frame >= first && frame < last) { pending.push_back({frame, in.isHolding, in.isPlayer1}); }
            }
        }
        else
        {
            for (auto& in : macro)
            {
                if (in.frame >= first && in.frame < last) { pending.push_back({in.frame, in.isHolding, in.isPlayer1}); }
            }
        }

        std::stable_sort(pending.begin(), pending.end(), [](const SimInput& a, const SimInput& b) { return a.frame < b.frame; });
    }

    size_t inputIndex = 0;

    bool finishedPlayer = false;
    bool finishedOther = false;

    int stepCountPlayer = 0;
    int stepCountOther = 0;

    for (int i = 0; i < iterations; i++)
    {
        if (playing)
        {
            const unsigned long long frame = g_frame + static_cast<unsigned long long>(i);
            while (inputIndex < pending.size() && pending[inputIndex].frame <= frame)
            {
                press(pending[inputIndex].isHolding, pending[inputIndex].isPlayer1);
                inputIndex++;
            }
        }

        if (!finishedPlayer)
        {
            finishedPlayer = this->iterate(pl, player, mode | playerMask, colors, stepCountPlayer);
        }
        if (count == 2 && !finishedOther)
        {
            finishedOther = this->iterate(pl, other, mode | otherMask, colors, stepCountOther);
        }

        if (finishedPlayer && (count == 1 || finishedOther)) { break; }

        for (auto& ac : autoclickers)
        {
            if (!ac.enabled) { continue; }

            if (ac.counter == 0)
            {
                press(true, ac.isPlayer1);
                if (ac.swift) { press(false, ac.isPlayer1); }
            }
            if (ac.counter == ac.holdFor)
            {
                if (ac.swift) { press(true, ac.isPlayer1); }
                press(false, ac.isPlayer1);
                ac.counter = -ac.releaseFor;
            }
            ac.counter++;
        }
    }
}

void Trajectory::simulate(GJBaseGameLayer* pl, bool p1, int mode, float* colors, bool both)
{
    PlayerObject* player = p1 ? m_fakePlayer1 : m_fakePlayer2;
    PlayerObject* realPlayer = p1 ? pl->m_player1 : pl->m_player2;
    PlayerObject* otherPlayer = p1 ? m_fakePlayer2 : m_fakePlayer1;
    PlayerObject* otherRealPlayer = p1 ? pl->m_player2 : pl->m_player1;

    if (!realPlayer) { return; }
    if (!otherRealPlayer) { both = false; }

    copyPlayerState(realPlayer, player);
    if (both) { copyPlayerState(otherRealPlayer, otherPlayer); }

    m_deadP1 = false;
    m_deadP2 = false;

    this->runPrediction(pl, player, otherPlayer, mode, colors, both);

    player->setVisible(false);
    otherPlayer->setVisible(false);

    m_activatedObjectsP1.clear();
    m_activatedObjectsP2.clear();

    gameStateSnapshot.restore(pl->m_gameState);
}

void Trajectory::update(GJBaseGameLayer* pl)
{
    if (!pl) { return; }

    m_fakePlayer1->setVisible(false);
    m_fakePlayer2->setVisible(false);

    if (!showTrajectory || replayStatus == 2)
    {
        if (m_calculated)
        {
            m_node->clear();
            m_calculated = false;
        }
        m_node->setVisible(false);
        return;
    }

    m_node->setVisible(true);

    Signature signature = computeSignature(pl);
    if (m_calculated && signature == m_lastSignature) { return; }

    const auto buildStart = std::chrono::steady_clock::now();
    if (!lockDelta && m_calculated && std::chrono::duration<double>(buildStart - m_lastBuild).count() < m_lastBuildCost * 4.0) { return; }

    m_drawing = true;
    m_node->clear();
    gameStateSnapshot.save(pl->m_gameState);
    m_delta = 60.0f / static_cast<float>(std::max(tps, 1));

    std::vector<std::pair<int, float*>> modes;
    if (holdTrajectory) { modes.push_back({TrajectoryMode::Hold, holdTrajectoryCol}); }
    if (swiftTrajectory) { modes.push_back({TrajectoryMode::Swift, swiftTrajectoryCol}); }
    if (releaseTrajectory) { modes.push_back({TrajectoryMode::Release, releaseTrajectoryCol}); }
    if (replayStatus == 1 && modes.size() > 1) { modes.resize(1); }

    const bool twoPlayer = pl->m_levelSettings->m_twoPlayerMode;

    if (pl->m_player1)
    {
        for (auto& [mode, colors] : modes)
        {
            simulate(pl, true, mode, colors, !twoPlayer);
        }
    }

    if (pl->m_player2 && pl->m_gameState.m_isDualMode && twoPlayer)
    {
        for (auto& [mode, colors] : modes)
        {
            simulate(pl, false, mode, colors, false);
        }
    }

    m_fakePlayer1->setVisible(false);
    m_fakePlayer2->setVisible(false);

    m_calculated = true;
    m_lastSignature = signature;
    m_drawing = false;

    const auto buildEnd = std::chrono::steady_clock::now();
    m_lastBuildCost = std::chrono::duration<double>(buildEnd - buildStart).count();
    m_lastBuild = buildEnd;
}

class $modify(TrajectoryPlayLayer, PlayLayer)
{
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects)
    {
        auto& trajectory = TrajectoryManager::get();
        if (trajectory.exists()) { trajectory.uninit(); }

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) { return false; }

        trajectory.init(this);

        return true;
    }

    void onQuit()
    {
        TrajectoryManager::get().uninit();
        PlayLayer::onQuit();
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();
        TrajectoryManager::get().invalidate();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object)
    {
        if (TrajectoryManager::get().hasDied(player)) { return; }
        PlayLayer::destroyPlayer(player, object);
    }
};

class $modify(TrajectoryGJBaseGameLayer, GJBaseGameLayer)
{
    void collisionCheckObjects(PlayerObject* player, gd::vector<GameObject*>* objects, int length, float dt)
    {
        if (!TrajectoryManager::get().isFakePlayer(player))
        {
            return GJBaseGameLayer::collisionCheckObjects(player, objects, length, dt);
        }

        phys::collisionCheckObjects(this, player, objects, length, dt);
    }

    void teleportPlayer(TeleportPortalObject* object, PlayerObject* player)
    {
        if (TrajectoryManager::get().isFakePlayer(player)) { phys::teleportPlayer(this, object, player); }
        else { GJBaseGameLayer::teleportPlayer(object, player); }
    }

    void flipGravity(PlayerObject* player, bool gravity, bool noEffects)
    {
        if (TrajectoryManager::get().isFakePlayer(player)) { phys::flipGravity(player, gravity); }
        else { GJBaseGameLayer::flipGravity(player, gravity, noEffects); }
    }

    void gameEventTriggered(GJGameEvent event, int p1, int p2)
    {
        if (TrajectoryManager::get().drawing()) { return; }
        GJBaseGameLayer::gameEventTriggered(event, p1, p2);
    }

    void destroyObject(GameObject* object)
    {
        if (TrajectoryManager::get().drawing()) { return; }
        GJBaseGameLayer::destroyObject(object);
    }
};

class $modify(TrajectoryPlayerObject, PlayerObject)
{
    void playSpiderDashEffect(cocos2d::CCPoint from, cocos2d::CCPoint to)
    {
        if (!TrajectoryManager::get().drawing()) { PlayerObject::playSpiderDashEffect(from, to); }
    }

    void incrementJumps()
    {
        if (!TrajectoryManager::get().drawing()) { PlayerObject::incrementJumps(); }
    }

    void ringJump(RingObject* ring, bool skipCheck)
    {
        if (TrajectoryManager::get().isFakePlayer(this)) { phys::ringJump(this, ring); }
        else { PlayerObject::ringJump(ring, skipCheck); }
    }

    void bumpPlayer(float force, int objectType, bool playEffect, GameObject* object)
    {
        if (TrajectoryManager::get().isFakePlayer(this)) { phys::bumpPlayer(this, force, objectType, playEffect, object); }
        else { PlayerObject::bumpPlayer(force, objectType, playEffect, object); }
    }

    void propellPlayer(float force, bool noEffects, int objectType)
    {
        if (TrajectoryManager::get().isFakePlayer(this)) { phys::propellPlayer(this, force, noEffects, objectType); }
        else { PlayerObject::propellPlayer(force, noEffects, objectType); }
    }

    void startDashing(DashRingObject* ring)
    {
        if (TrajectoryManager::get().isFakePlayer(this)) { phys::startDashing(this, ring); }
        else { PlayerObject::startDashing(ring); }
    }

    void tryPlaceCheckpoint()
    {
        if (TrajectoryManager::get().isFakePlayer(this)) { return; }
        PlayerObject::tryPlaceCheckpoint();
    }

    void removePendingCheckpoint()
    {
        if (TrajectoryManager::get().isFakePlayer(this)) { return; }
        PlayerObject::removePendingCheckpoint();
    }

#ifdef GEODE_IS_WINDOWS
    void stopDashing()
    {
        if (TrajectoryManager::get().isFakePlayer(this)) { phys::stopDashing(this); }
        else { PlayerObject::stopDashing(); }
    }
#endif
};

class $modify(TrajectoryEnhancedGameObject, EnhancedGameObject)
{
    void activatedByPlayer(PlayerObject* player)
    {
        if (TrajectoryManager::get().isFakePlayer(player)) { phys::activateForTrajectory((EffectGameObject*)this, player); }
        else { EnhancedGameObject::activatedByPlayer(player); }
    }
};

class $modify(TrajectoryCCScheduler, cocos2d::CCScheduler)
{
    static void onModify(auto& self)
    {
        (void)self.setHookPriorityPre("cocos2d::CCScheduler::update", Priority::First);
    }

    void update(float dt)
    {
        CCScheduler::update(dt);

        auto pl = PlayLayer::get();
        if (!pl || pl->m_isPaused) { return; }

        TrajectoryManager::get().update(pl);
    }
};
