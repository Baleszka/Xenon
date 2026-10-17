#pragma once

inline bool trajectory = false;
inline bool holdTrajectory = true;
inline bool releaseTrajectory = true;

inline float trajectoryTrailWidth = 0.3f;
inline int trajectoryLength = 300;

inline float holdTrajectoryCol[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
inline float releaseTrajectoryCol[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

inline bool simulating = false;
inline bool fakeDied = false;
