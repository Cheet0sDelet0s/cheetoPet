#include "graphics.h"

void drawBitmapFromList(int16_t x, int16_t y, int dir, const uint8_t bitmapID, uint16_t color) {
  const BitmapInfo& bmp = bitmaps[bitmapID];

  drawBitmapWithDirection(x, y, dir, bmp.data, bmp.width, bmp.height, color);
}

void drawBitmapWithDirection(int16_t x, int16_t y, int dir, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
  if (dir == 1) {
    display.drawBitmap(x, y, bitmap, w, h, color);
  } else {
    drawBitmapFlippedX(x, y, bitmap, w, h, color);
  }
}

void drawBitmapFlippedX(int16_t x, int16_t y,
                                const uint8_t *bitmap, int16_t w, int16_t h,
                                uint16_t color) {
  int byteWidth = (w + 7) / 8; // Bitmap width in bytes

  for (int16_t j = 0; j < h; j++) {
    for (int16_t i = 0; i < w; i++) {
      int16_t flipped_i = w - 1 - i;
      uint8_t byte = pgm_read_byte(bitmap + j * byteWidth + (flipped_i / 8));
      if (byte & (0x80 >> (flipped_i % 8))) {
        display.drawPixel(x + i, y + j, color);
      }
    }
  }
}