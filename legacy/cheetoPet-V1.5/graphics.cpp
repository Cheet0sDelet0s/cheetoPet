#include "graphics.h"

#define MAX_PARTICLES 50

Particle particles[MAX_PARTICLES];
int particleCount = 0;

int currentMenuItem = 0;
int menuScroll = 0;

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

void drawCenteredText(const String &text, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  
  // Get the size of the text
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  
  // Calculate the centered X position
  int16_t x = (display.width() - w) / 2;

  // Draw the text at the centered X position
  display.setCursor(x, y);
  display.print(text);
}

void drawTextCenteredX(const char *text, int16_t centerX, int16_t y) {  // draw text centered to specified x y coords
  int16_t x1, y1;
  uint16_t w, h;

  // Measure the text size
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);

  // Calculate the left-aligned starting X
  int16_t startX = centerX - (w / 2);

  // Draw the text
  display.setCursor(startX, y);
  display.print(text);
}

void showPopup(String text, int time) {
  display.setTextColor(SH110X_BLACK, SH110X_WHITE);
  display.setTextSize(1);
  drawCenteredText(text, 60);
  display.display();
  delay(time);
  display.setTextColor(SH110X_WHITE);
}

void drawTextRightAligned(int16_t y, String text) {
  int16_t x1, y1;
  uint16_t w, h;

  // Measure text bounds at (0, 0)
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // Calculate right-aligned X position
  int16_t x = display.width() - w;

  // Draw text
  display.setCursor(x, y);
  display.print(text);
}

void drawCheckerboard(uint8_t squareSize) {
  bool color;
  for (uint8_t y = 0; y < 128; y += squareSize) {
    color = (y / squareSize) % 2;  // Alternate row start
    for (uint8_t x = 0; x < 128; x += squareSize) {
      display.fillRect(x, y, squareSize, squareSize, color ? SH110X_WHITE : SH110X_BLACK);
      color = !color;  // Alternate color for each square
      }
    display.display();
  }
  display.display();
  delay(300);
}

void spiralFill(uint16_t color) {
  int x0 = 0;
  int y0 = 0;
  int x1 = SCREEN_WIDTH - 1;
  int y1 = SCREEN_HEIGHT - 1;

  while (x0 <= x1 && y0 <= y1) {
    // Top edge
    for (int x = x0; x <= x1; x++) {
      display.drawPixel(x, y0, color);
    }
    y0++;

    // Right edge
    for (int y = y0; y <= y1; y++) {
      display.drawPixel(x1, y, color);
    }
    x1--;

    // Bottom edge
    if (y0 <= y1) {
      for (int x = x1; x >= x0; x--) {
        display.drawPixel(x, y1, color);
      }
      y1--;
    }

    // Left edge
    if (x0 <= x1) {
      for (int y = y1; y >= y0; y--) {
        display.drawPixel(x0, y, color);
      }
      x0++;
    }
    display.display();
    delay(5);  // Adjust delay for animation speed
  }
}

void drawLiveData() {
  display.setTextColor(SH110X_WHITE);
  display.setFont(&Picopixel);
  DateTime now = rtc.now();
  String data = "";
  data += String(now.hour());
  data += ":";
  if (now.minute() < 10) {
    data += "0";
    data += String(now.minute());
  } else {
    data += String(now.minute());
  }
  drawTextRightAligned(5, data);
  data = String(now.day()); data += "/"; data += String(now.month());
  drawTextRightAligned(11, data);

  int status = getBatteryStatus();

  if (status == 0) { // discharging
    data = String(currentBatteryPercentage) + "%";
  } else if (status == 1) { // charging
    data = "CHARGE";
  } else if (status == 2) { // finished charging
    data = "FINISH";
  } else if (status == 3) { // error
    data = "BAT ERROR";
  }

  drawTextRightAligned(17, data);
  drawTextRightAligned(23, String(currentBatteryVoltage) + "V");

  display.setFont(NULL);
}

void drawMenu(Menu* menu, uint8_t menuLength, const String& menuName) {
  const uint8_t ROW_HEIGHT = 10;
  const uint8_t HEADER_HEIGHT = 16;
  const uint8_t VISIBLE_ROWS = (SCREEN_HEIGHT - HEADER_HEIGHT) / ROW_HEIGHT;

  drawLiveData();

  if (currentMenuItem < 0) { // wrap selected item
    currentMenuItem = menuLength - 1;
  } else if (currentMenuItem == menuLength) {
    currentMenuItem = 0;
  }

  if (currentMenuItem < menuScroll) {
    menuScroll = currentMenuItem;
  } else if (currentMenuItem >= menuScroll + VISIBLE_ROWS) {
    menuScroll = currentMenuItem - VISIBLE_ROWS + 1;
  }

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Header
  display.setCursor(0, 0);
  display.println(menuName);

  // Draw visible menu entries
  for (uint8_t i = 0; i < VISIBLE_ROWS; i++) {
    uint8_t menuItemIndex = menuScroll + i;
    if (menuItemIndex >= menuLength) break;

    display.setCursor(0, HEADER_HEIGHT + (i * ROW_HEIGHT));

    if (menuItemIndex == currentMenuItem) {
      display.print("> ");
    } else {
      display.print("  ");
    }

    display.println(menu->items[menuItemIndex].name);
  }

  // Scroll indicator
  if (menuScroll > 0) {
    display.fillTriangle(120, 18, 124, 18, 122, 14, SH110X_WHITE);
  }
  if (menuScroll + VISIBLE_ROWS < menuLength) {
    display.fillTriangle(120, 120, 124, 120, 122, 124, SH110X_WHITE);
  }
}

void drawInventory() {
  const uint8_t ROW_HEIGHT = 10;
  const uint8_t HEADER_HEIGHT = 16;
  const uint8_t VISIBLE_ROWS = ((SCREEN_HEIGHT - HEADER_HEIGHT) / ROW_HEIGHT);

  const uint8_t TOTAL_ITEMS = inventoryItems + 1; // +1 for "Back"

  drawLiveData();

  if (currentMenuItem < 0) {
    currentMenuItem = TOTAL_ITEMS - 1;
  } else if (currentMenuItem >= TOTAL_ITEMS) {
    currentMenuItem = 0;
  }

  if (currentMenuItem < menuScroll) {
    menuScroll = currentMenuItem;
  } else if (currentMenuItem >= menuScroll + VISIBLE_ROWS) {
    menuScroll = currentMenuItem - VISIBLE_ROWS + 1;
  }

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Header
  display.setCursor(0, 0);
  display.println("inventory");

  // Draw visible menu entries
  for (uint8_t i = 0; i < VISIBLE_ROWS; i++) {
    uint8_t menuItemIndex = menuScroll + i;
    if (menuItemIndex >= TOTAL_ITEMS) break;

    display.setCursor(0, HEADER_HEIGHT + (i * ROW_HEIGHT));

    if (menuItemIndex == currentMenuItem) {
      display.print("> ");
    } else {
      display.print("  ");
    }

    if (menuItemIndex == 0) {
      display.println("back");
    } else {
      display.println(displayNames[inventory[menuItemIndex - 1]]);
    }
  }

  // Scroll indicators
  if (menuScroll > 0) {
    display.fillTriangle(120, 18, 124, 18, 122, 14, SH110X_WHITE);
  }
  if (menuScroll + VISIBLE_ROWS < TOTAL_ITEMS) {
    display.fillTriangle(120, 120, 124, 120, 122, 124, SH110X_WHITE);
  }
}


// PARTICLES

void updateParticles() {
    for (int i = 0; i < particleCount; i++) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].lifetime--;
    
        // Remove particle if its lifetime is over
        if (particles[i].lifetime <= 0) {
        // Shift remaining particles down
        for (int j = i; j < particleCount - 1; j++) {
            particles[j] = particles[j + 1];
        }
        particleCount--;
        i--; // Adjust index to account for removed particle
        }
    }
}

void drawParticles() {
    for (int i = 0; i < particleCount; i++) {
        int type = particles[i].type;

        switch (type) {
            case 1: {    // dot particle
                display.drawPixel((int)particles[i].x, (int)particles[i].y, SH110X_WHITE);
                break;
            }
            case 2: {    // circle particle
                display.drawCircle((int)particles[i].x, (int)particles[i].y, 1, SH110X_WHITE);
                break;
            }
            case 3: {    // line particle
                int x = particles[i].x;
                int y = particles[i].y;
                float vx = particles[i].vx;   // keep as float
                float vy = particles[i].vy;   // keep as float
                int life = particles[i].lifetime;
                int maxLife = particles[i].maxlife;
            
                // Scale the line length based on remaining lifetime
                float scale = (float)life / maxLife;
                int scaledVx = (int)(vx * scale * 4);
                int scaledVy = (int)(vy * scale * 4);
            
                display.drawLine(x, y, x + scaledVx, y + scaledVy, SH110X_WHITE);
                break;
            }
            
        }
    }
}

void createParticle(int type, float x, float y, float vx, float vy, int lifetime) {
    if (particleCount < MAX_PARTICLES) {
        particles[particleCount].type = type;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = vx;
        particles[particleCount].vy = vy;
        particles[particleCount].lifetime = lifetime;
        particles[particleCount].maxlife = lifetime;
        particleCount++;
    }
}

void clearParticles() {
    particleCount = 0;
}