#pragma once

struct fVector4
{
    float x1;
    float y1;
    float x2;
    float y2;
};

struct fVector2
{
    float x;
    float y;
};

struct dVector2
{
    double x;
    double y;
};

fVector4 getPlayerPos();
dVector2 getPlayerVel();
fVector2 getPlayerRot();
int getRandomSeed();