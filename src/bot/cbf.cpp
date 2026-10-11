#include "cbf.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

bool midStep = false;
bool p1Split = false;
bool p2Split = false;

float rotationDelta = 0.f;

CCPoint p1Pos;
CCPoint p2Pos;

// only split if the click wouldn't just get buffered
bool canAct(PlayerObject* p) 
{
    return p->m_isOnGround || p->m_touchingRings->count() || p->m_isDashing || p->m_isDart || p->m_isBird || p->m_isShip || p->m_isSwing;
}

void advance(PlayLayer* pl, PlayerObject* p, float d, float stepDelta) 
{
    p->update(d);
    pl->checkCollisions(p, (p->m_isOnSlope && !p->m_isDart) ? stepDelta : 0.0f, true);
    p->updateRotation(d);
    p->m_collisionLogTop->removeAllObjects();
    p->m_collisionLogBottom->removeAllObjects();
    p->m_collisionLogLeft->removeAllObjects();
    p->m_collisionLogRight->removeAllObjects();
    p->m_lastCollisionLeft = p->m_lastCollisionRight = -1;
    p->m_lastCollisionBottom = p->m_lastCollisionTop = -1;
}

class $modify(PlayerObject) 
{
    void update(float dt) 
    {
        auto pl = PlayLayer::get();
        if (midStep || !pl) { PlayerObject::update(dt); return; }

        if (this != pl->m_player1) 
        {
            // P2 gets updated inside P1's call when this step is split
            if (!(this == pl->m_player2 && !stepInputs.empty())) { PlayerObject::update(dt); }
            return;
        }

        if (stepInputs.empty()) 
        {
            p1Split = p2Split = false;
            PlayerObject::update(dt);
            return;
        }

        PlayerObject* p2 = pl->m_player2;

        p1Split = canAct(this);
        p2Split = pl->m_gameState.m_isDualMode && canAct(p2);

        p1Pos = getPosition();
        p2Pos = p2->getPosition();

        midStep = true;
        double prev = 0.0;

        for (auto& s : stepInputs)
        { 
            float d = dt * float(s.alpha - prev);

            if (p1Split) { advance(pl, this, d, dt); }
            if (p2Split) { advance(pl, p2, d, dt); }

            pl->handleButton(s.isHolding, (int)PlayerButton::Jump, s.isPlayer1);
            prev = s.alpha;
        }

        rotationDelta = dt * float(1.0 - prev);
        PlayerObject::update(p1Split ? rotationDelta : dt);
        p2->update(p2Split ? rotationDelta : dt);
        midStep = false;
    }

    // vanilla calls this after update(); finish the rotation and restore m_lastPosition
    void updateRotation(float t) 
    {
        auto pl = PlayLayer::get();
        if (!midStep && pl && this == pl->m_player1 && p1Split) 
        {
            PlayerObject::updateRotation(rotationDelta);
            m_lastPosition = p1Pos;
        }
        else if (!midStep && pl && this == pl->m_player2 && p2Split) 
        {
            PlayerObject::updateRotation(rotationDelta);
            m_lastPosition = p2Pos;
        }
        else { PlayerObject::updateRotation(t); }
    }
};

// Borrowed from CBF code