#pragma once

#include <Geode/Geode.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_set>

using namespace geode::prelude;

inline bool showTrajectory = false;
inline bool holdTrajectory = true;
inline bool releaseTrajectory = true;
inline bool swiftTrajectory = false;

inline float trajectoryTrailWidth = 0.5f;
inline int trajectoryLength = 240;

inline float holdTrajectoryCol[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
inline float releaseTrajectoryCol[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
inline float swiftTrajectoryCol[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

namespace TrajectoryMode
{
    enum : int
    {
        Hold = 0x1,
        Swift = 0x2,
        Release = 0x4,
        Player1 = 0x20,
        Player2 = 0x40
    };

    constexpr int ClickMask = 0x7;
}

class TrajectoryDrawNode : public cocos2d::CCDrawNode
{
public:
    static TrajectoryDrawNode* create()
    {
        TrajectoryDrawNode* node = new TrajectoryDrawNode();

        if (node && node->init())
        {
            node->autorelease();
            node->m_bUseArea = false;
        }
        else
        {
            CC_SAFE_DELETE(node);
        }

        return node;
    }
};

class Trajectory
{
public:
    PlayerObject* m_fakePlayer1 = nullptr;
    PlayerObject* m_fakePlayer2 = nullptr;

private:
    struct Signature
    {
        unsigned long long frame = 0;
        float p1[7] = {};
        uint64_t p1flags = 0;
        float p2[7] = {};
        uint64_t p2flags = 0;
        float timeWarp = 0.0f;
        float cameraZoom = 0.0f;
        float width = 0.0f;
        float hitboxWidth = 0.0f;
        int length = 0;
        int tps = 0;
        int status = 0;
        int autoclicker[6] = {};
        uint32_t boolPack = 0;
        size_t colorHash = 0;
        size_t macroSize = 0;

        bool operator==(const Signature&) const = default;
    };

    TrajectoryDrawNode* m_node = nullptr;

    std::unordered_set<uintptr_t> m_activatedObjectsP1;
    std::unordered_set<uintptr_t> m_activatedObjectsP2;

    Signature m_lastSignature;

    bool m_calculated = false;
    bool m_drawing = false;
    bool m_deadP1 = false;
    bool m_deadP2 = false;

    float m_delta = 0.25f;

    std::chrono::steady_clock::time_point m_lastBuild;
    double m_lastBuildCost = 0.0;

    Trajectory() = default;

    static PlayerObject* createFakePlayer(GJBaseGameLayer* pl, const std::string& id);

    Signature computeSignature(GJBaseGameLayer* pl);
    bool iterate(GJBaseGameLayer* pl, PlayerObject* player, int mode, float* colors, int& stepCount);
    void runPrediction(GJBaseGameLayer* pl, PlayerObject* player, PlayerObject* other, int mode, float* colors, bool both);
    void simulate(GJBaseGameLayer* pl, bool p1, int mode, float* colors, bool both);
    void drawHitbox(PlayerObject* player);

public:
    static Trajectory* create(GJBaseGameLayer* pl);
    ~Trajectory();

    bool drawing() const { return m_drawing; }

    bool isFakePlayer(PlayerObject* player) const
    {
        return player == m_fakePlayer1 || player == m_fakePlayer2;
    }

    PlayerObject* getOtherPlayer(PlayerObject* player) const
    {
        if (player == m_fakePlayer1) { return m_fakePlayer2; }
        return m_fakePlayer1;
    }

    bool hasDied(PlayerObject* player)
    {
        if (player == m_fakePlayer1)
        {
            m_deadP1 = true;
            return true;
        }

        if (player == m_fakePlayer2)
        {
            m_deadP2 = true;
            return true;
        }

        return false;
    }

    void invalidateCache() { m_calculated = false; }

    void update(GJBaseGameLayer* pl);

    bool playerHasActivated(PlayerObject* player, EnhancedGameObject* object);
    bool realPlayerHasActivated(PlayerObject* player, EnhancedGameObject* object);

    void rememberActivatedObject(EnhancedGameObject* object, PlayerObject* player)
    {
        if (player == m_fakePlayer1) { m_activatedObjectsP1.insert((uintptr_t)object); }
        else if (player == m_fakePlayer2) { m_activatedObjectsP2.insert((uintptr_t)object); }
    }
};

class TrajectoryManager
{
private:
    Trajectory* m_trajectory = nullptr;

    TrajectoryManager() = default;

public:
    static TrajectoryManager& get()
    {
        static TrajectoryManager instance;
        return instance;
    }

    bool exists() const { return m_trajectory != nullptr; }

    bool drawing() const { return m_trajectory && m_trajectory->drawing(); }

    bool isFakePlayer(PlayerObject* player) const
    {
        return m_trajectory && m_trajectory->isFakePlayer(player);
    }

    bool isFakePlayer2(PlayerObject* player) const
    {
        return m_trajectory && m_trajectory->m_fakePlayer2 == player;
    }

    PlayerObject* getOtherPlayer(PlayerObject* player) const
    {
        return m_trajectory ? m_trajectory->getOtherPlayer(player) : nullptr;
    }

    bool hasDied(PlayerObject* player)
    {
        return m_trajectory && m_trajectory->hasDied(player);
    }

    bool playerHasActivated(PlayerObject* player, EnhancedGameObject* object)
    {
        return m_trajectory && m_trajectory->playerHasActivated(player, object);
    }

    bool realPlayerHasActivated(PlayerObject* player, EnhancedGameObject* object)
    {
        return m_trajectory && m_trajectory->realPlayerHasActivated(player, object);
    }

    void rememberActivatedObject(EnhancedGameObject* object, PlayerObject* player)
    {
        if (m_trajectory) { m_trajectory->rememberActivatedObject(object, player); }
    }

    void update(GJBaseGameLayer* pl)
    {
        if (m_trajectory) { m_trajectory->update(pl); }
    }

    void invalidate()
    {
        if (m_trajectory) { m_trajectory->invalidateCache(); }
    }

    void init(GJBaseGameLayer* pl)
    {
        m_trajectory = Trajectory::create(pl);
    }

    void uninit()
    {
        delete m_trajectory;
        m_trajectory = nullptr;
    }
};
