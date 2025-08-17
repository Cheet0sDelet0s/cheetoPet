#ifndef PONG_H
#define PONG_H

#include "bitmaps.h"

extern float petPongXP;
extern int petPongLVL;

extern Adafruit_SH1107 display;

extern float angleY;
extern void petMessage(String message);
extern float money;
extern int petFun;
extern bool leftButtonState;
extern bool rightButtonState;
extern void updateButtonStates();
extern void updateGyro();
extern void drawCheckerboard(uint8_t squareSize);
extern void drawCenteredText(Adafruit_GFX &display, const String &text, int16_t y);
extern void audioEngine();
extern void queueTone(float freq, int length);
extern void priorityQueueTone(int freq);
extern void clearTones();
//extern const BitmapInfo bitmaps;

void drawPetLeveling(String levelType, float beginningXP, float gainedXP, int beginningLVL);

void stepBallForward();

void reEnergizeBall(float amount);

void pong();

#endif