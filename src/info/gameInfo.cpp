#include "gameInfo.hpp"

using namespace geode::prelude;

fVector4 getPlayerPos()
{
    auto playLayer = PlayLayer::get();
    auto player1 = playLayer->m_player1;
    
    if (playLayer->m_gameState.m_isDualMode)
    {
        auto player2 = playLayer->m_player2;
        return fVector4{player1->getPositionX(), player1->getPositionY(), player2->getPositionX(), player2->getPositionY()};
    }
    else
    {
        return fVector4{player1->getPositionX(), player1->getPositionY(), 0.0f, 0.0f};
    }
    
}

dVector2 getPlayerVel()
{
    auto playLayer = PlayLayer::get();
    auto player1 = playLayer->m_player1;
    if (playLayer->m_gameState.m_isDualMode)
    {
        auto player2 = playLayer->m_player2;
        return dVector2{player1->getYVelocity(), player2->getYVelocity()};
    }
    else
    {
        return dVector2{player1->getYVelocity(), 0.0f};
    }
    
}

fVector2 getPlayerRot()
{
    auto playLayer = PlayLayer::get();
    auto player1 = playLayer->m_player1;
    if (playLayer->m_gameState.m_isDualMode)
    {
        auto player2 = playLayer->m_player2;
        return fVector2{player1->getRotationX(), player2->getRotationX()};
    }
    else
    {
        return fVector2{player1->getRotationX(), 0.0f};
    }
}

int getRandomSeed()
{
    auto gjbgl = GJBaseGameLayer::get();
    return gjbgl->m_randomSeed;
}

int getFrame()
{
    auto gjbgl = GJBaseGameLayer::get();
    return gjbgl->m_gameState.m_currentProgress / 2;
}