#pragma once

#include <vector>

using namespace geode::prelude;

inline bool showHitboxes = false;
inline bool hitboxTrail = false;
inline float hitboxWidth = 0.25f;
inline int trailLength = 256;

inline float hazardCol[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
inline float solidCol[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
inline float interactableCol[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
inline float hazardRotCol[4] = { 0.33f, 0.0f, 0.0f, 1.0f };
inline float passableCol[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

inline bool showPlayer = true;
inline bool showPlayerRot = true;
inline bool showPlayerInner = true;
inline bool showPlayerCircle = true;

void drawPlayerHitbox(cocos2d::CCDrawNode* node, GJBaseGameLayer* gl, PlayerObject* player);
void drawPlayerHitboxOtherOne(cocos2d::CCDrawNode* node, GJBaseGameLayer* pl, std::array<cocos2d::CCPoint, 4> corners, cocos2d::CCRect rect, float radius, cocos2d::CCPoint pos);
void syncToObjectLayer(CCDrawNode* node, GJBaseGameLayer* pl);

struct playerHitbox
{
    std::array<cocos2d::CCPoint, 4> corners;
    cocos2d::CCRect rect;
    float radius;
    cocos2d::CCPoint pos;
    bool player1;
};

inline std::vector<playerHitbox> hitboxTrailArr;