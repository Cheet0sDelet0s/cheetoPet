#ifndef LATINPROJECT_H
#define LATINPROJECT_H

#include <Arduino.h>

extern Adafruit_SH1107 display;

extern bool rightButtonState;
extern void updateButtonStates();
extern void drawCenteredText(Adafruit_GFX &display, const String &text, int16_t y);
extern void waitForSelectRelease();

void latinProject();

#endif