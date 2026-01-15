/* ---- graphics.h ----

  holds a bunch of useful Adafruit_GFX drawing functions (made by me) used all across the UI

*/
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <functional>

// ---- CHEETOPET FILES ----
#include <hardware.h>
#include <bitmaps.h>

struct Particle {
  int type, lifetime, maxlife;
  float x, y;
  float vx, vy;
};

struct MenuItem {
  const char* name;
  std::function<void()> action;
};

struct Menu {
  MenuItem* items;
  uint8_t length;
  const char* name;
};

extern int currentMenuItem;
extern int menuScroll;

void drawBitmapFromList(int16_t x, int16_t y, int dir, const uint8_t bitmapID, uint16_t color);
void drawBitmapWithDirection(int16_t x, int16_t y, int dir, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
void drawBitmapFlippedX(int16_t x, int16_t y,
                                const uint8_t *bitmap, int16_t w, int16_t h,
                                uint16_t color);
void drawCenteredText(const String &text, int16_t y);
void drawTextCenteredX(const char *text, int16_t centerX, int16_t y);
void showPopup(String text, int time);
void spiralFill(uint16_t color);

void drawMenu(Menu* menu, uint8_t menuLength, const String& menuName);

// particle system
void updateParticles();
void drawParticles();
void createParticle(int type, float x, float y, float vx, float vy, int lifetime);

#endif
