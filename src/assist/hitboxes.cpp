#include "hitboxes.hpp"
#include "Geode/cocos/draw_nodes/CCDrawNode.h"
#include "Geode/cocos/CCDirector.h"
#include "../bot/replay.hpp"

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

void drawPlayerHitbox(cocos2d::CCDrawNode* node, GJBaseGameLayer* pl, PlayerObject* player)
{
    auto borderWidth = hitboxWidth / pl->m_gameState.m_cameraZoom;

    if (showPlayerRot) // draw this first so normal hitbox is over rotated
    {
        auto rectRot = player->getOrientedBox();
        node->drawPolygon(rectRot->m_corners.data(), 4, {0.0f, 0.0f, 0.0f, 0.0f}, borderWidth, {hazardRotCol[0], hazardRotCol[1], hazardRotCol[2], hazardRotCol[3]}, cocos2d::BorderAlignment::Inside);
    }
    if (showPlayer) 
    {
        auto rect = player->getObjectRect();
        node->drawRect(rect, {0.0f, 0.0f, 0.0f, 0.0f}, borderWidth, {hazardCol[0], hazardCol[1], hazardCol[2], hazardCol[3]}, cocos2d::BorderAlignment::Inside);
    }
    if (showPlayerInner) 
    {
        auto inner = player->getObjectRect(0.3, 0.3); 
        node->drawRect(inner, {0.0f, 0.0f, 0.0f, 0.0f}, borderWidth, {solidCol[0], solidCol[1], solidCol[2], solidCol[3]}, cocos2d::BorderAlignment::Inside);
    }
    if (showPlayerCircle) 
    {
        float radius = player->getObjectRect().size.width / 2.0;
        u_int segments = (u_int)(std::max(radius, 8.0f) * 2.0f * sqrt(pl->m_gameState.m_cameraZoom));
        node->drawCircle(player->getPosition(), radius, {0.0f, 0.0f, 0.0f, 0.0f}, hitboxWidth, {hazardCol[0], hazardCol[1], hazardCol[2], hazardCol[3]}, segments);
    }
}

void drawPlayerHitboxOtherOne(cocos2d::CCDrawNode* node, GJBaseGameLayer* pl, std::array<cocos2d::CCPoint, 4> corners, cocos2d::CCRect rect, float radius, cocos2d::CCPoint pos)
{
    auto borderWidth = hitboxWidth / pl->m_gameState.m_cameraZoom;

    if (showPlayerRot) // draw this first so normal hitbox is over rotated
    {
        node->drawPolygon(corners.data(), 4, {0.0f, 0.0f, 0.0f, 0.0f}, borderWidth, {hazardRotCol[0], hazardRotCol[1], hazardRotCol[2], hazardRotCol[3]}, cocos2d::BorderAlignment::Inside);
    }
    if (showPlayer) 
    {
        node->drawRect(rect, {0.0f, 0.0f, 0.0f, 0.0f}, borderWidth, {hazardCol[0], hazardCol[1], hazardCol[2], hazardCol[3]}, cocos2d::BorderAlignment::Inside);
    }
    if (showPlayerInner) 
    {
        float w = rect.size.width * 0.3f;
        float h = rect.size.height * 0.3f;
        auto inner = cocos2d::CCRect(rect.getMidX() - w / 2.0f, rect.getMidY() - h / 2.0f, w, h);
        node->drawRect(inner, {0.0f, 0.0f, 0.0f, 0.0f}, borderWidth, {solidCol[0], solidCol[1], solidCol[2], solidCol[3]}, cocos2d::BorderAlignment::Inside);
    }
    if (showPlayerCircle) 
    {
        u_int segments = (u_int)(std::max(radius, 8.0f) * 2.0f * sqrt(pl->m_gameState.m_cameraZoom));
        node->drawCircle(pos, radius, {0.0f, 0.0f, 0.0f, 0.0f}, hitboxWidth, {hazardCol[0], hazardCol[1], hazardCol[2], hazardCol[3]}, segments);
    }
}

void drawLevelHitboxes(cocos2d::CCDrawNode* node, GJBaseGameLayer* pl, GameObject* object)
{
    auto borderWidth = hitboxWidth / pl->m_gameState.m_cameraZoom;
    bool isEditor = pl->m_isEditor;

    if (object->m_isGroupDisabled || !object->m_isActivated || object->m_objectType == GameObjectType::Decoration) { return; }
    if ((object == pl->m_player1 || object == pl->m_player2)) { return; }
    if (isEditor) { object->updateStartValues(); }

    auto makeColor = [](float* color) {
        return cocos2d::ccColor4F{color[0], color[1], color[2], color[3]};
    };

    switch(object->m_objectType)
    {
        default:
        {
            if (object->hasBeenActivatedByPlayer(pl->m_player1) && (!pl->m_gameState.m_isDualMode || object->hasBeenActivatedByPlayer(pl->m_player2))) { return; }
            if (object == pl->m_player1CollisionBlock || object == pl->m_player2CollisionBlock) { return; }

            bool rectDirty = object->m_isObjectRectDirty;
            bool boxOffsetCalculated = object->m_boxOffsetCalculated;
            auto color = makeColor(interactableCol);
            if (auto orientedBox = isEditor ? object->getOrientedBox() : object->m_orientedBox; orientedBox)
            {
                node->drawPolygon(orientedBox->m_corners.data(), 4, {0, 0, 0, 0}, borderWidth, color, cocos2d::BorderAlignment::Inside);
            }
            else
            {
                node->drawRect(object->getObjectRect(), {0, 0, 0, 0}, borderWidth, color, cocos2d::BorderAlignment::Inside);
            }
            object->m_isObjectRectDirty = rectDirty;
            object->m_boxOffsetCalculated = boxOffsetCalculated;
            break;
        }

        case GameObjectType::Solid:
        {
            auto color = makeColor(object->m_isPassable ? passableCol : solidCol);
            node->drawRect(object->getObjectRect(), {0, 0, 0, 0}, borderWidth, color, cocos2d::BorderAlignment::Inside);
            break;
        }

        case GameObjectType::Slope:
        {
            auto rect = object->getObjectRect();
            cocos2d::CCPoint verts[] = {
                rect.origin,
                {rect.getMinX(), rect.getMaxY()},
                {rect.getMaxX(), rect.getMinY()},
            };

            auto max = cocos2d::CCPoint{rect.getMaxX(), rect.getMaxY()};
            switch (object->m_slopeDirection)
            {
                case 0:
                case 7:
                    verts[1] = max;
                    break;
                case 1:
                case 5:
                    verts[0] = max;
                    break;
                case 3:
                case 6:
                    verts[2] = max;
                    break;
            }

            auto color = makeColor(object->m_isPassable ? passableCol : solidCol);
            node->drawPolygon(verts, 3, {0, 0, 0, 0}, borderWidth, color, cocos2d::BorderAlignment::Inside);
            break;
        }

        case GameObjectType::AnimatedHazard:
        case GameObjectType::Hazard:
        {
            if (object == pl->m_anticheatSpike) { return; }

            bool rectDirty = object->m_isObjectRectDirty;
            bool boxOffsetCalculated = object->m_boxOffsetCalculated;
            auto color = makeColor(hazardCol);
            float radius = object->m_objectRadius * std::max(object->m_scaleX, object->m_scaleY);
            if (radius > 0)
            {
                u_int segments = (u_int)(std::max(radius, 8.0f) * 2.0f * sqrt(pl->m_gameState.m_cameraZoom));
                node->drawCircle(object->getPosition(), radius, {0, 0, 0, 0}, borderWidth, color, segments);
            }
            else if (auto orientedBox = isEditor ? object->getOrientedBox() : object->m_orientedBox; orientedBox)
            {
                node->drawPolygon(orientedBox->m_corners.data(), 4, {0, 0, 0, 0}, borderWidth, makeColor(hazardRotCol), cocos2d::BorderAlignment::Inside);
            }
            else
            {
                node->drawRect(object->getObjectRect(), {0, 0, 0, 0}, borderWidth, color, cocos2d::BorderAlignment::Inside);
            }
            object->m_isObjectRectDirty = rectDirty;
            object->m_boxOffsetCalculated = boxOffsetCalculated;
            break;
        }
    }

}

void iterateObjects(GJBaseGameLayer* pl, std::function<void(GameObject*)> callback) {
    auto& sections = pl->m_sections;

    for (int i = pl->m_leftSectionIndex; i <= pl->m_rightSectionIndex && static_cast<size_t>(i) < sections.size(); i++) 
    {
        auto horizontalSection = sections[i];
        if (!horizontalSection) continue;

        for (int j = pl->m_bottomSectionIndex; j <= pl->m_topSectionIndex && static_cast<size_t>(j) < horizontalSection->size(); j++) 
        {
            auto section = horizontalSection->at(j);
            if (!section) { continue; }

            for (int k = 0; k < pl->m_sectionSizes[i]->at(j); k++) 
            {
                auto object = section->at(k);
                if (!object) { continue; }
                callback(object);
            }
        }
    }
}

void syncToObjectLayer(CCDrawNode* node, GJBaseGameLayer* pl)
{
    auto parent = node->getParent();
    if (!parent || !pl->m_objectLayer) { return; }

    auto layerToWorld = pl->m_objectLayer->nodeToWorldTransform();
    auto worldToParent = parent->worldToNodeTransform();
    node->setAdditionalTransform(cocos2d::CCAffineTransformConcat(layerToWorld, worldToParent));
}

class $modify(HitboxStepLayer, GJBaseGameLayer)
{
    void processCommands(float dt, bool isHalfTick, bool isLastTick)
    {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);

        if (!showHitboxes || !hitboxTrail || m_isEditor || !m_player1) { return; }

        hitboxTrailArr.push_back({m_player1->getOrientedBox()->m_corners, m_player1->getObjectRect(), m_player1->getObjectRect().size.width / 2.0f, m_player1->getPosition(), true});
        if (m_gameState.m_isDualMode && m_player2)
        {
            hitboxTrailArr.push_back({m_player2->getOrientedBox()->m_corners, m_player2->getObjectRect(), m_player2->getObjectRect().size.width / 2.0f, m_player2->getPosition(), false});
        }

        size_t maxSize = trailLength * (m_gameState.m_isDualMode ? 2 : 1);
        while (hitboxTrailArr.size() > maxSize)
        {
            hitboxTrailArr.erase(hitboxTrailArr.begin());
        }
    }
};

class $modify(HitboxPlayLayer, PlayLayer) 
{
    struct Fields 
    {
        CCDrawNode* drawNode = nullptr; 
        CCDrawNode* playerDrawNode = nullptr;
    };

    void postUpdate(float dt) 
    {
        PlayLayer::postUpdate(dt);

        if (!showHitboxes || replayStatus == 2) 
        {
            if (m_fields->drawNode) { m_fields->drawNode->clear(); m_fields->playerDrawNode->clear(); }
            return;
        }

        if (!m_fields->drawNode)
        {
            m_fields->drawNode = CCDrawNode::create();
            addChild(m_fields->drawNode, m_objectLayer->getZOrder() + 1);
        }

        if (!m_fields->playerDrawNode)
        {
            m_fields->playerDrawNode = CCDrawNode::create();
            addChild(m_fields->playerDrawNode, m_objectLayer->getZOrder() + 1);
        }

        auto node = m_fields->drawNode;
        auto playerNode = m_fields->playerDrawNode;
        syncToObjectLayer(node, this);
        syncToObjectLayer(playerNode, this);
        node->clear();

        iterateObjects(this, [&](GameObject* object) { drawLevelHitboxes(node, this, object); });

        if (hitboxTrail && showHitboxes)
        {
            playerNode->clear();
            for (int i = 0; i < hitboxTrailArr.size(); i++)
            {
                drawPlayerHitboxOtherOne(playerNode, this, hitboxTrailArr[i].corners, hitboxTrailArr[i].rect, hitboxTrailArr[i].radius, hitboxTrailArr[i].pos);
            }
        }
        else
        {
            hitboxTrailArr.clear();
            playerNode->clear();
            drawPlayerHitbox(playerNode, this, m_player1);
            if (m_gameState.m_isDualMode) { drawPlayerHitbox(playerNode, this, m_player2); }
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        hitboxTrailArr.clear();
        if (m_fields->drawNode) { m_fields->drawNode->clear(); }
        if (m_fields->playerDrawNode) { m_fields->playerDrawNode->clear(); }
    }

};

// parts of the code are borrowed from silicate