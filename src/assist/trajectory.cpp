#include "trajectory.hpp"
#include "hitboxes.hpp"
#include "../bot/tpsBypass.hpp"

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/HardStreak.hpp>

using namespace geode::prelude;

// type: 0 hold 1 release
void simulate(PlayLayer* pl, CCDrawNode* node, PlayerObject* fake, PlayerObject* p, int type, float* col)
{
    // start the fake one exactly where the real player is
    fake->copyAttributes(p);
    fake->setPosition(p->m_position);
    fake->setRotation(p->getRotation());
    fake->m_playEffects = false; // no particles
    fake->m_maybeReducedEffects = true;

    if (type == 0) fake->pushButton(PlayerButton::Jump);
    else {
        fake->releaseButton(PlayerButton::Jump);
        fake->m_jumpBuffered = false; // or it still jumps if you clicked a frame before
    }

    float dt = getPhysicsDt() * 60.f * pl->m_gameState.m_timeWarp;
    float w = trajectoryTrailWidth / pl->m_gameState.m_cameraZoom;

    ccColor4F c = {col[0], col[1], col[2], 1.0f};
    if (p == pl->m_player2) c = {1.f - col[0], 1.f - col[1], 1.f - col[2], 1.f}; // p2 gets the inverted color

    fakeDied = false;

    for (int i = 0; i < trajectoryLength; i++) 
    {
        auto last = fake->getPosition();

        // same as advance() in cbf.cpp otherwise it breask
        fake->update(dt);
        pl->checkCollisions(fake, (fake->m_isOnSlope && !fake->m_isDart) ? dt : 0.0f, false);
        fake->updateRotation(dt);
        fake->m_collisionLogTop->removeAllObjects();
        fake->m_collisionLogBottom->removeAllObjects();
        fake->m_collisionLogLeft->removeAllObjects();
        fake->m_collisionLogRight->removeAllObjects();

        if (fakeDied) { drawPlayerHitbox(node, pl, fake); break; } // show where you would die

        node->drawSegment(last, fake->getPosition(), w, c);
        if (fake->getPositionX() > pl->m_levelLength) break; 
    }
    fake->setVisible(false);
}

class $modify(TrajectoryPL, PlayLayer) 
{
    struct Fields {
        Ref<CCDrawNode> node;
        Ref<PlayerObject> fake1;
        Ref<PlayerObject> fake2;
    };

    // invisible copies of the player that get moved around for the sim
    PlayerObject* makeFake() {
        auto p = PlayerObject::create(1, 1, this, this, true);
        p->setPosition({0, 105});
        p->setVisible(false);
        m_objectLayer->addChild(p);
        return p;
    }

    void postUpdate(float dt) 
    {
        PlayLayer::postUpdate(dt);

        if (!m_fields->node)
        {
            m_fields->node = CCDrawNode::create();
            addChild(m_fields->node, m_objectLayer->getZOrder() + 2); // above hitboxes
            m_fields->fake1 = makeFake();
            m_fields->fake2 = makeFake();
        }
        CCDrawNode* node = m_fields->node;
        node->clear();

        if (!trajectory || !m_player1 || m_playerDied) return;
        syncToObjectLayer(node, this);

        // all the hooks below check this so the fake players dont trigger anything
        simulating = true;

        if (holdTrajectory) simulate(this, node, m_fields->fake1, m_player1, 0, holdTrajectoryCol);
        if (releaseTrajectory) simulate(this, node, m_fields->fake1, m_player1, 1, releaseTrajectoryCol);

        if (m_gameState.m_isDualMode)
        {
            if (holdTrajectory) simulate(this, node, m_fields->fake2, m_player2, 0, holdTrajectoryCol);
            if (releaseTrajectory) simulate(this, node, m_fields->fake2, m_player2, 1, releaseTrajectoryCol);
        }

        simulating = false;
    }

    void destroyPlayer(PlayerObject* p, GameObject* obj) {
        if (simulating) { fakeDied = true; return; }
        PlayLayer::destroyPlayer(p, obj);
    }

    void levelComplete() { if (!simulating) PlayLayer::levelComplete(); }
    void playEndAnimationToPos(CCPoint pos) { if (!simulating) PlayLayer::playEndAnimationToPos(pos); }
};

class $modify(TrajectoryBGL, GJBaseGameLayer)
{
    bool canBeActivatedByPlayer(PlayerObject* p, EffectGameObject* obj) 
    {
        if (!simulating) return GJBaseGameLayer::canBeActivatedByPlayer(p, obj);

        // fake player cant activate stuff so do the portals by hand
        // TODO pads orbs and gamemode portals
        switch (obj->m_objectID) {
            case 10: if (p->m_isUpsideDown) p->flipGravity(false, true); break;
            case 11: if (!p->m_isUpsideDown) p->flipGravity(true, true); break;
            case 99: if (p->m_vehicleSize != 1.f) p->togglePlayerScale(false, true); break;
            case 101: if (p->m_vehicleSize == 1.f) p->togglePlayerScale(true, true); break;
            // speed
            case 200: p->m_playerSpeed = 0.7f; break;
            case 201: p->m_playerSpeed = 0.9f; break;
            case 202: p->m_playerSpeed = 1.1f; break;
            case 203: p->m_playerSpeed = 1.3f; break;
            case 1334: p->m_playerSpeed = 1.6f; break;
        }
        return false;
    }

    void playerTouchedRing(PlayerObject* p, RingObject* obj) { if (!simulating) GJBaseGameLayer::playerTouchedRing(p, obj); }
    void playerTouchedTrigger(PlayerObject* p, EffectGameObject* obj) { if (!simulating) GJBaseGameLayer::playerTouchedTrigger(p, obj); }
    void playerWillSwitchMode(PlayerObject* p, GameObject* obj) { if (!simulating) GJBaseGameLayer::playerWillSwitchMode(p, obj); }
    void gameEventTriggered(GJGameEvent e, int material, int id) { if (!simulating) GJBaseGameLayer::gameEventTriggered(e, material, id); }
};

// no sounds/effects/jump counter from the fake players
class $modify(PlayerObject) 
{
    void incrementJumps() { if (!simulating) PlayerObject::incrementJumps(); }
    void playSpiderDashEffect(CCPoint from, CCPoint to) { if (!simulating) PlayerObject::playSpiderDashEffect(from, to); }
    void spawnPortalCircle(ccColor3B col, float r) { if (!simulating) PlayerObject::spawnPortalCircle(col, r); }
};

// wave trail
class $modify(HardStreak) {
    void addPoint(CCPoint pt) { if (!simulating) HardStreak::addPoint(pt); }
};

// some of this is from looking at silicate's trajectory but NOT copied wallahi

// also wave is a lil wierd but i actually have no clue why 
