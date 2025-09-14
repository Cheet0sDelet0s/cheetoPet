#ifndef CHEETOENGINE_H
#define CHEETOENGINE_H

#include <Arduino.h>

extern Adafruit_SH1107 display;
extern bool leftButtonState;
extern bool rightButtonState;
extern void updateButtonStates();
extern void updateGyro();
extern void waitForSelectRelease();
extern float angleX;
extern float angleY;
extern float angleZ;

struct Point3D {
    float x, y, z;
};

void cheetoEngine();

#endif