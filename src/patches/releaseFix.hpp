#pragma once

#include <Geode/Geode.hpp>
#include <vector>

using namespace geode::prelude;

// PlayerObject::releaseAllButtons from 0x14039f100 to 0x14039f2c6 (454 bytes)
// 
// 12 xrefs:
// > GJBaseGameLayer::toggleDualMode @ 0x140216c1f
// > GJBaseGameLayer::resetLevelVariables @ 0x14023b4f1 <-
// > GJBaseGameLayer::resetLevelVariables @ 0x14023b4fd <-
// > LevelEditorLayer::onStopPlaytest @ 0x1402e090d
// > PlayerObject::loadFromCheckpoint @ 0x1403a31e0
// > PlayerObject::loadFromCheckpoint @ 0x1403a3222
// > PlayLayer::showEndLayer @ 0x1403ac737
// > PlayLayer::showRetryLayer @ 0x1403ac932
// > PlayLayer::pauseGame @ 0x1403ba239 <-
// > PlayLayer::pauseGame @ 0x1403ba27c <-
// > PlayLayer::resumeAndRestart @ 0x1403ba508 <-
// > PlayLayer::resume @ 0x1403ba879 <-
// ^^^ WIN DECOMP ^^^
// (originally said 18 xrefs but last 4 were useless lel)
// from https://gd-src.undefined0.dev/

void applyPatches();
void disablePatches();

extern bool patchesEnabled;

$execute
{
    // GJBaseGameLayer::resetLevelVariables @ 0x14023b4f1 && GJBaseGameLayer::resetLevelVariables @ 0x14023b4fd
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x23B4EA), // offset
            std::vector<uint8_t>(24, 0x90))) { // fill 24 bytes with NOP (no operation)
        geode::log::error("Failed to patch GJBaseGameLayer::resetLevelVariables");
    }
    else { geode::log::info("Succesfully patched GJBaseGameLayer::resetLevelVariables"); }

    // > PlayLayer::resumeAndRestart @ 0x1403ba508
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x3BA501), // offset
            std::vector<uint8_t>(12, 0x90))) { // fill 12 bytes with NOP (no operation)
        geode::log::error("Failed to patch PlayLayer::resumeAndRestart");
    }
    else { geode::log::info("Succesfully patched PlayLayer::resumeAndRestart"); }
    
    // PlayLayer::resume @ 0x1403ba879
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x3BA872), // offset
            std::vector<uint8_t>(12, 0x90))) { // fill 12 bytes with NOP (no operation)
        geode::log::error("Failed to patch PlayLayer::resume");
    }
    else { geode::log::info("Succesfully patched PlayLayer::resume"); }

    // PlayLayer::pauseGame @ 0x1403ba239
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x3BA232), // offset
            std::vector<uint8_t>(12, 0x90))) { // fill 12 bytes with NOP (no operation)
        geode::log::error("Failed to patch PlayLayer::pauseGame");
    }
    else { geode::log::info("Succesfully patched PlayLayer::pauseGame"); }

    // PlayLayer::pauseGame @ 0x1403ba27c
    if (!Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + 0x3BA275), // offset
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
//
// Offsets for 2.209:
// GJBaseGameLayer::resetLevelVariables 0x342A3C BYTES->24 NOP
// PlayLayer::resumeAndRestart 0x516C41 BYTES->12 NOP
// PlayLayer::resume 0x516C41 BYTES->12 NOP
// PlayLayer::pauseGame 0x5168B1 BYTES->12 NOP
// PlayLayer::pauseGame#2 0x516907 BYTES->12 NOP

// Offsets for 2.2091:
// GJBaseGameLayer::resetLevelVariables 0x34549C BYTES->24 NOP
// PlayLayer::resumeAndRestart 0x51A8A1 BYTES->12 NOP
// PlayLayer::resume 0x51AC7E BYTES->12 NOP
// PlayLayer::pauseGame 0x51A511 BYTES->12 NOP
// PlayLayer::pauseGame#2 0x51A567 BYTES->12 NOP