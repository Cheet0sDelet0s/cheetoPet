/* veridium - cheetoPet C++ port by olly jeffery

based on the original game made in scratch

a 2d shooter game

*/
#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "veridium.h"

#define MAX_BULLETS 50  // max bullets on screen

DRAM_ATTR float petVeridiumXP = 0;
DRAM_ATTR int petVeridiumLVL = 1;
DRAM_ATTR bool stopVeridium = false;

DRAM_ATTR int playerX = 10;
DRAM_ATTR int playerY = 10;
DRAM_ATTR int gunXA = 0;
DRAM_ATTR int gunYA = 0;
DRAM_ATTR int gunXB = 0;
DRAM_ATTR int gunYB = 0;
DRAM_ATTR int playerSpeed = 2;

struct Bullet {
  float x, y;
  float vx, vy;
  bool active;
};

Bullet bullets[MAX_BULLETS];

// Call this to spawn a bullet from the gun position with velocity based on angle
void spawnBullet(float startX, float startY, float angle) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = startX;
      bullets[i].y = startY;
      float speed = 5.0;  // adjust bullet speed here
      bullets[i].vx = speed * cos(angle);
      bullets[i].vy = -speed * sin(angle);  // minus because screen Y-axis flips
      bullets[i].active = true;
      break;
    }
  }
}

// Update all bullets position and deactivate if off screen
void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].x += bullets[i].vx;
      bullets[i].y += bullets[i].vy;

      // Remove bullet if outside screen bounds
      if (bullets[i].x < 0 || bullets[i].x > 127 || bullets[i].y < 0 || bullets[i].y > 127) {
        bullets[i].active = false;
      }
    }
  }
}

// Draw all active bullets on screen
void drawBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      //display.fillCircle((int)bullets[i].x, (int)bullets[i].y, 2, SH110X_WHITE);
      display.drawPixel((int)bullets[i].x, (int)bullets[i].y, SH110X_WHITE);
    }
  }
}


float getAngleBetweenPoints(int x1, int y1, int x2, int y2) {
  int dx = x2 - x1;
  int dy = y1 - y2;  // Flip Y for screen coordinates
  return atan2((float)dy, (float)dx);
}


void veridium() {  //launch veridium

  while (!stopVeridium) {

    updateGyro();
    cursorX = angleX;
    cursorY = angleY;

    updateButtonStates();

    float angle = getAngleBetweenPoints(playerX, playerY, angleX, angleY);

    if (rightButtonState) {
      playerX += playerSpeed * cos(angle);
      playerY -= playerSpeed * sin(angle);
    }

    if (leftButtonState) {
      angleX = 64;
      angleY = 64;
    }

    if (middleButtonState) {
      spawnBullet(gunXB, gunYB, angle);
    }

    gunXA = playerX;
    gunYA = playerY;

    gunXA += 8 * cos(angle);
    gunYA -= 8 * sin(angle);

    gunXB = gunXA;
    gunYB = gunYA;

    gunXB += 4 * cos(angle);
    gunYB -= 4 * sin(angle);

    updateBullets();

    display.clearDisplay();
    // draw player and gun as before
    display.drawCircle(playerX, playerY, 4, SH110X_WHITE);
    display.drawLine(gunXA, gunYA, gunXB, gunYB, SH110X_WHITE);

    //display.fillCircle(angleX, angleY, 2, SH110X_WHITE);

    display.drawFastHLine(angleX - 2, angleY, 5, SH110X_WHITE);
    display.drawFastVLine(angleX, angleY - 2, 5, SH110X_WHITE);

    // draw bullets
    drawBullets();

    display.fillRect(110, 0, 17, 17, SH110X_WHITE);
    display.drawLine(111, 1, 126, 16, SH110X_BLACK);
    display.drawLine(126, 1, 111, 16, SH110X_BLACK);
    if (detectCursorTouch(110, 0, 17, 17) && rightButtonState) {
      stopVeridium = true;
    }
    display.display();
    delay(10);

  }
  stopVeridium = false;
}