/* ---- graphics.h ----

  holds a bunch of useful Adafruit_GFX drawing functions (made by me) used all across the UI

*/
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ---- CHEETOPET FILES ----
#include <hardware.h>
#include <bitmaps.h>

void drawBitmapFromList(int16_t x, int16_t y, int dir, const uint8_t bitmapID, uint16_t color);
void drawBitmapWithDirection(int16_t x, int16_t y, int dir, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
void drawBitmapFlippedX(int16_t x, int16_t y,
                                const uint8_t *bitmap, int16_t w, int16_t h,
                                uint16_t color);

#endif
