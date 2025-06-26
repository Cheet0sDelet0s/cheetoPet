#ifndef FLAPPYBIRD_H
#define FLAPPYBIRD_H

#include <Arduino.h>

extern Adafruit_SH1107 display;

extern bool leftButtonState;
extern bool rightButtonState;
extern void updateButtonStates();
extern void drawPetLeveling(String levelType, float beginningXP, float gainedXP, int beginningLVL);
extern float money;
extern int petFun;
extern void petMessage(String message);
void flappyBird();

#endif