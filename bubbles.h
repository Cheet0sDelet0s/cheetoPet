#ifndef BUBBLES_H
#define BUBBLES_H

#include <Arduino.h>

extern Adafruit_SH1107 display;

extern bool leftButtonState;
extern bool middleButtonState;
extern bool rightButtonState;
extern bool previousLeftState;
extern bool previousMiddleState;
extern void updateButtonStates();
extern void updateGyro();
extern float angleX;
extern float angleY;
extern void drawPetLeveling(String levelType, float beginningXP, float gainedXP, int beginningLVL);
extern float money;
extern int petFun;
extern void petMessage(String message);
extern void screenRecord();
extern void audioEngine();
extern void queueTone(float freq, int length);
extern void priorityQueueTone(int freq);
extern void clearTones();
void bubbleSim();

struct Bubble {
  float x, y;
  float vx, vy;
};

#endif