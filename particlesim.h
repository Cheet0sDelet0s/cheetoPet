#ifndef PARTICLESIM_H
#define PARTICLESIM_H

#include <Arduino.h>

extern Adafruit_SH1107 display;

extern bool leftButtonState;
extern bool rightButtonState;
extern void updateButtonStates();
extern void updateGyro();
extern float angleX;
extern float angleY;
extern void drawPetLeveling(String levelType, float beginningXP, float gainedXP, int beginningLVL);
extern float money;
extern int petFun;
extern void petMessage(String message);
void particleSim();

struct Particle {
  float x, y;
  float vx, vy;
};

#endif