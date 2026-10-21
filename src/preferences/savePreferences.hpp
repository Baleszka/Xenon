#pragma once

#include <Geode/Geode.hpp>
#include <matjson.hpp>
#include <array>
#include "../assist/hitboxes.hpp"

struct hitboxSaveData
{
    float hitboxWidth = 0.25f;
    int trailLength = 256;

    std::array<float, 4> hazardCol = { 1.f, 0.f, 0.f, 1.f };
    std::array<float, 4> solidCol = { 0.f, 0.f, 1.f, 1.f };
    std::array<float, 4> interactableCol = { 1.f, 1.f, 0.f, 1.f };
    std::array<float, 4> hazardRotCol = { 0.33f, 0.f, 0.f, 1.f };
    std::array<float, 4> passableCol = { 0.f, 1.f, 1.f, 1.f };
    
    bool showPlayer = true;
    bool showPlayerRot = true;
    bool showPlayerInner = true;
    bool showPlayerCircle = true;
};

template <>
struct matjson::Serialize<hitboxSaveData>
{
    static Result<hitboxSaveData> fromJson(Value const& value)
    {
        GEODE_UNWRAP_INTO(float hitboxWidth, value["hitboxWidth"].as<float>());
        GEODE_UNWRAP_INTO(int trailLength, value["trailLength"].as<int>());
        GEODE_UNWRAP_INTO(auto hazardCol, value["hazardCol"].as<std::array<float, 4>>());
        GEODE_UNWRAP_INTO(auto solidCol, value["solidCol"].as<std::array<float, 4>>());
        GEODE_UNWRAP_INTO(auto interactableCol, value["interactableCol"].as<std::array<float, 4>>());
        GEODE_UNWRAP_INTO(auto hazardRotCol, value["hazardRotCol"].as<std::array<float, 4>>());
        GEODE_UNWRAP_INTO(auto passableCol, value["passableCol"].as<std::array<float, 4>>());
        GEODE_UNWRAP_INTO(bool showPlayer, value["showPlayer"].as<bool>());
        GEODE_UNWRAP_INTO(bool showPlayerRot, value["showPlayerRot"].as<bool>());
        GEODE_UNWRAP_INTO(bool showPlayerInner, value["showPlayerInner"].as<bool>());
        GEODE_UNWRAP_INTO(bool showPlayerCircle, value["showPlayerCircle"].as<bool>());

        return geode::Ok(hitboxSaveData
        {
            hitboxWidth, trailLength,
            hazardCol, solidCol, interactableCol, hazardRotCol, passableCol,
            showPlayer, showPlayerRot, showPlayerInner, showPlayerCircle
        });
    }

    static Value toJson(hitboxSaveData const& d)
    {
        return matjson::makeObject
        ({
            { "hitboxWidth", d.hitboxWidth },
            { "trailLength", d.trailLength },
            { "hazardCol", d.hazardCol },
            { "solidCol", d.solidCol },
            { "interactableCol", d.interactableCol },
            { "hazardRotCol", d.hazardRotCol },
            { "passableCol", d.passableCol },
            { "showPlayer", d.showPlayer },
            { "showPlayerRot", d.showPlayerRot },
            { "showPlayerInner", d.showPlayerInner },
            { "showPlayerCircle", d.showPlayerCircle }
        });
    }
};

struct replaySaveData
{
    bool speedhackAudio = true;
    bool blockInputs = true;
};

template <>
struct matjson::Serialize<replaySaveData>
{
    static Result<replaySaveData> fromJson(Value const& value)
    {
        GEODE_UNWRAP_INTO(bool speedhackAudio, value["speedhackAudio"].as<bool>());
        GEODE_UNWRAP_INTO(bool blockInputs, value["blockInputs"].as<bool>());

        return geode::Ok(replaySaveData
        {
            speedhackAudio, blockInputs
        });
    }

    static Value toJson(replaySaveData const& d)
    {
        return matjson::makeObject
        ({
            { "speedhackAudio", d.speedhackAudio },
            { "blockInputs", d.blockInputs }
        });
    }
};