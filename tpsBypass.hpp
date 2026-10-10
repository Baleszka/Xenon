#pragma once

#include "replay.hpp"

using namespace geode::prelude;

inline double m_tpsOverflow = 0.0;

inline int m_respawnTimer = 0;
inline int totalStepCount = 0;
inline int estimatedStepCount = 1;

inline bool lockDelta = true;
inline bool lockDeltaStep = false;
inline double lockDeltaOverflow = 0.0;

inline bool isPaused() 
{
	auto pl = PlayLayer::get();
	return pl && pl->m_isPaused;
}
inline double getPhysicsDt() { return 1. / tps; }
extern void calculateSteps(float dt, float targetDt);