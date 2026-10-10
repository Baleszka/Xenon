#pragma once

extern double firstFrame;
extern double lastFrame;
extern int tpsCalc;

double calcFramesAvailable(double firstFrame, double lastFrame);
double calcTimeAvailable(double framesAvailable, int tps);
double calcFramePerfectOn(double framesAvailable, int tps);