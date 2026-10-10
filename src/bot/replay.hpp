#pragma once

#include <vector>

struct input
{
    unsigned long long frame;
    bool isHolding;
    bool isPlayer1;
};

struct cbfInput
{
    double preciseFrame;
    bool isHolding;
    bool isPlayer1;
};

inline std::vector<input> macro;
inline std::vector<cbfInput> cbfMacro;

inline int tps = 240;
inline int replayStatus = 2; // 0 -> record 1 -> replay 2 -> disable

extern bool ignoreInputs;

void addInput(unsigned long long frame, bool isHolding, bool isPlayer1);
void simulateInput(bool isHolding, bool isPlayer1);