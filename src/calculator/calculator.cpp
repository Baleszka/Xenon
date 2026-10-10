#include "calculator.hpp"

double firstFrame;
double lastFrame;
int tpsCalc = 240;

double calcFramesAvailable(double firstFrame, double lastFrame)
{
    return lastFrame-firstFrame;
}

double calcTimeAvailable(double framesAvailable, int tps)
{
    return framesAvailable / tps;
}

double calcFramePerfectOn(double framesAvailable, int tps)
{
    return tps / framesAvailable;
}