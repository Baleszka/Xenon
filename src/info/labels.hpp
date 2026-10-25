#pragma once

#include <string>

using namespace geode::prelude;

struct label
{
    std::string text;
    int position; // 0->TL 1->TR 2->BL 3->BR
    float opacity;
    int size;
    bool enabled;
};

inline bool labelsEnabled = false;

inline label positionLabels = { "", 0, 1.0f, 10, false};

inline label xPositionLabel = 
{
    "X",
    positionLabels.position, 
    positionLabels.opacity, 
    positionLabels.size, 
    positionLabels.enabled 
};

inline label yPositionLabel = 
{
    "Y",
    positionLabels.position, 
    positionLabels.opacity, 
    positionLabels.size, 
    positionLabels.enabled 
};

inline void syncPositionLabels()
{
    for (label* l : { &xPositionLabel, &yPositionLabel })
    {
        l->position = positionLabels.position;
        l->opacity  = positionLabels.opacity;
        l->size     = positionLabels.size;
        l->enabled  = positionLabels.enabled;
    }
}

inline label velocityLabel = { "Velocity", 0, 1.0f, 10, false };
inline label rotationLabel = { "Rotation", 0, 1.0f, 10, false };

inline label tickLabel = { "Tick", 0, 1.0f, 10, false };
inline label seedLabel = { "Seed", 0, 1.0f, 10, false };

inline cocos2d::CCPoint getAnchorPointForCorner(int corner)
{
    if (corner == 0) { return { 0.0f, 1.0f }; }
    if (corner == 1) { return { 1.0f, 1.0f }; }
    if (corner == 2) { return { 0.0f, 0.0f }; }
    if (corner == 3) { return { 1.0f, 0.0f }; }
    return {};
}

inline float cornerOffset[4] = { 0.f, 0.f, 0.f, 0.f };

inline void resetLabelOffsets()
{
    for (int i = 0; i < 4; i++)
    {
        cornerOffset[i] = 0.f;
    }
}

inline void applyLabel(CCLabelBMFont* node, label const& l)
{
    if (!node) return;

    auto win = CCDirector::get()->getWinSize();
    float pad = 2.5f;
    float spacing = 2.f * (l.size / 10.f);

    bool right = (l.position == 1 || l.position == 3);
    bool top = (l.position == 0 || l.position == 1);

    bool visible = labelsEnabled && l.enabled;

    node->setVisible(visible);
    node->setAnchorPoint(getAnchorPointForCorner(l.position));
    node->setScale(l.size / 10.f);
    node->setOpacity(static_cast<GLubyte>(l.opacity * 255));

    if (!visible) return;

    float offset = cornerOffset[l.position];
    
    float x = right ? win.width - pad : pad;
    float y = top ? win.height - pad - offset : pad + offset;

    node->setPosition({ x, y });

    float height = node->getContentSize().height * node->getScale();
    cornerOffset[l.position] += height + spacing;
}