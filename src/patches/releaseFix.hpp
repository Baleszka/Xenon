#pragma once

#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

void applyPatches();
void disablePatches();

extern bool patchesEnabled;

$execute
{
    // GJBaseGameLayer::resetLevelVariables
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x34549C), // offset
            std::vector<uint8_t>(24, 0x90))) { // fill 24 bytes with NOP (no operation)
        geode::log::error("Failed to patch GJBaseGameLayer::resetLevelVariables");
    }
    else { geode::log::info("Succesfully patched GJBaseGameLayer::resetLevelVariables"); }

    // PlayLayer::resumeAndRestart
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x51A8A1), // offset
            std::vector<uint8_t>(12, 0x90))) { // fill 12 bytes with NOP (no operation)
        geode::log::error("Failed to patch PlayLayer::resumeAndRestart");
    }
    else { geode::log::info("Succesfully patched PlayLayer::resumeAndRestart"); }
    
    // PlayLayer::resume
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x51AC7E), // offset
            std::vector<uint8_t>(12, 0x90))) { // fill 12 bytes with NOP (no operation)
        geode::log::error("Failed to patch PlayLayer::resume");
    }
    else { geode::log::info("Succesfully patched PlayLayer::resume"); }

    // PlayLayer::pauseGame
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x51A511), // offset
            std::vector<uint8_t>(12, 0x90))) { // fill 12 bytes with NOP (no operation)
        geode::log::error("Failed to patch PlayLayer::pauseGame");
    }
    else { geode::log::info("Succesfully patched PlayLayer::pauseGame"); }

    // PlayLayer::pauseGame
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x51A567), // offset
            std::vector<uint8_t>(12, 0x90))) { // fill 12 bytes with NOP (no operation)
        geode::log::error("Failed to patch PlayLayer::pauseGame other one");
    }
    else { geode::log::info("Succesfully patched PlayLayer::pauseGame other one"); } 
    
    disablePatches();
}

// Resources:
// 
// https://gd-src.undefined0.dev/ <- Decompiled GD code
// IDA Pro <- Checking stuff
// https://www.calculator.net/hex-calculator.html <- Calculating with hex values
//
// Notes bc i'll forget lel:
// Offset = Address - Base
// Address = Base + Offset
// 
// Base = 140000000 because GD is 64bit