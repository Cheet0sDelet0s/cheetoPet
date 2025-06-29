#ifndef VERIDIUM_H
#define VERIDIUM_H

#include "bitmaps.h"

extern Adafruit_SH1107 display;

extern float petVeridiumXP;
extern int petVeridiumLVL;

extern float angleX;
extern float angleY;
extern int cursorX;
extern int cursorY;
//extern void petMessage(String message);
//extern float money;
//extern int petFun;
extern bool leftButtonState;
extern bool middleButtonState;
extern bool rightButtonState;
extern void updateButtonStates();
extern void updateGyro();
extern bool detectCursorTouch(int startX, int startY, int endX, int endY);

void veridium();

#endif