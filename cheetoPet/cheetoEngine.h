#ifndef CHEETOENGINE_H
#define CHEETOENGINE_H

#include <Arduino.h>

extern Adafruit_SH1107 display;
extern bool leftButtonState;
extern bool middleButtonState;
extern bool rightButtonState;
extern void updateButtonStates();
extern void updateGyro();
extern void waitForSelectRelease();
extern float angleX;
extern float angleY;
extern float angleZ;

extern bool drawAdjustable(int x, int y, int& value, int minVal, int maxVal, const char* label, bool selected);
extern void drawCenteredText(const String &text, int16_t y);
extern void updateCursor();
extern void drawCursor();
extern void updateParticles();
extern void drawParticles();
extern bool drawButton(int x, int y, int w, int h, const char* label);
void cheetoEngine();

#endif