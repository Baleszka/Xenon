#include "labels.hpp"
#include "gameInfo.hpp"

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(LabelPlayLayer, PlayLayer)
{
    struct Fields 
    {
        CCLabelBMFont* m_xPositionLabel = nullptr;
        CCLabelBMFont* m_yPositionLabel = nullptr;
        CCLabelBMFont* m_velocityLabel = nullptr;
        CCLabelBMFont* m_rotationLabel = nullptr;
        CCLabelBMFont* m_tickLabel = nullptr;
        CCLabelBMFont* m_seedLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool dontCreateObjects) 
    {
        if (!PlayLayer::init(level, dontCreateObjects)) return false;

        auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();

        auto xPosLabel = CCLabelBMFont::create(xPositionLabel.text.c_str(), "chatFont.fnt");
        auto yPosLabel = CCLabelBMFont::create(yPositionLabel.text.c_str(), "chatFont.fnt");
        auto velLabel = CCLabelBMFont::create(velocityLabel.text.c_str(), "chatFont.fnt");
        auto rotLabel = CCLabelBMFont::create(rotationLabel.text.c_str(), "chatFont.fnt");
        auto tckLabel = CCLabelBMFont::create(tickLabel.text.c_str(), "chatFont.fnt");
        auto sdLabel = CCLabelBMFont::create(seedLabel.text.c_str(), "chatFont.fnt");

        xPosLabel->setZOrder(100);
        yPosLabel->setZOrder(100);
        velLabel->setZOrder(100);
        rotLabel->setZOrder(100);
        tckLabel->setZOrder(100);
        sdLabel->setZOrder(100);

        this->addChild(xPosLabel);
        this->addChild(yPosLabel);
        this->addChild(velLabel);
        this->addChild(rotLabel);
        this->addChild(tckLabel);
        this->addChild(sdLabel);

        m_fields->m_xPositionLabel = xPosLabel;
        m_fields->m_yPositionLabel = yPosLabel;
        m_fields->m_velocityLabel = velLabel;
        m_fields->m_rotationLabel = rotLabel;
        m_fields->m_tickLabel = tckLabel;
        m_fields->m_seedLabel = sdLabel;

        return true;
    }

    void postUpdate(float dt) 
    {
        PlayLayer::postUpdate(dt);
        
        resetLabelOffsets();
        syncPositionLabels();
        applyLabel(m_fields->m_xPositionLabel, xPositionLabel);
        applyLabel(m_fields->m_yPositionLabel, yPositionLabel);
        applyLabel(m_fields->m_velocityLabel, velocityLabel);
        applyLabel(m_fields->m_rotationLabel, rotationLabel);

        if (!m_gameState.m_isDualMode)
        {
            m_fields->m_xPositionLabel->setString(fmt::format("{}: {:.6f}", xPositionLabel.text, getPlayerPos().x1).c_str());
            m_fields->m_yPositionLabel->setString(fmt::format("{}: {:.6f}", xPositionLabel.text, getPlayerPos().y1).c_str());
            m_fields->m_velocityLabel->setString(fmt::format("{}: {:.3f}", velocityLabel.text, getPlayerVel().x).c_str());
            m_fields->m_rotationLabel->setString(fmt::format("{}: {:.6f}", rotationLabel.text, getPlayerRot().x).c_str());
        }
        else
        {
            m_fields->m_xPositionLabel->setString(fmt::format("{}: {:.6f} / {:.6f}", xPositionLabel.text, getPlayerPos().x1, getPlayerPos().x2).c_str());
            m_fields->m_yPositionLabel->setString(fmt::format("{}: {:.6f} / {:.6f}", xPositionLabel.text, getPlayerPos().y1, getPlayerPos().y2).c_str());
            m_fields->m_velocityLabel->setString(fmt::format("{}: {:.3f} / {:.3f}", velocityLabel.text, getPlayerVel().x, getPlayerVel().y).c_str());
            m_fields->m_rotationLabel->setString(fmt::format("{}: {:.6f} / {:.6f}", rotationLabel.text, getPlayerRot().x, getPlayerRot().y).c_str());
        }
        

        applyLabel(m_fields->m_tickLabel, tickLabel);
        m_fields->m_tickLabel->setString(fmt::format("{}: {}", tickLabel.text, getFrame()).c_str());

        applyLabel(m_fields->m_seedLabel, seedLabel);
        m_fields->m_seedLabel->setString(fmt::format("{}: {}", seedLabel.text, getRandomSeed()).c_str());
    }
};