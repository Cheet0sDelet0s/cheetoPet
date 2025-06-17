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
DRAM_ATTR int playerSpeed = 4;
DRAM_ATTR int playerAmmo = 50;
const int maxAmmo = 50;

DRAM_ATTR bool isReloading = false;
DRAM_ATTR unsigned long reloadStartTime = 0;
DRAM_ATTR const unsigned long reloadDuration = 2000; // 2 seconds


struct Bullet {
  float x, y;
  float vx, vy;
  bool active;
};

struct Wall {
  int xa, ya, xb, yb;
};

struct Enemy {
  int x, y;
  int health;
  bool active;
};

const int maxEnemies = 5;
DRAM_ATTR Enemy enemies[maxEnemies];

const float enemySpeed = 0.5;  // same speed for all

Bullet bullets[MAX_BULLETS];

const int wallCount = 5;
Wall walls[wallCount] = {
  {20, 20, 30, 30},
  {60, 60, 80, 70},
  {10, 60, 20, 90},
  {55, 100, 75, 110},
  {10, 66, 14, 88}
};

void drawWalls() {
  for (int i = 0; i < wallCount; i++) {
    Wall currentWall = walls[i];
    int width = abs(currentWall.xb - currentWall.xa);
    int height = abs(currentWall.yb - currentWall.ya);
    display.drawRoundRect(currentWall.xa, currentWall.ya, width, height, 2, SH110X_WHITE);
  }
}

bool isInsideWall(int x, int y) {
  for (int i = 0; i < wallCount; i++) {
    Wall w = walls[i];
    int left = min(w.xa, w.xb);
    int right = max(w.xa, w.xb);
    int top = min(w.ya, w.yb);
    int bottom = max(w.ya, w.yb);
    
    if (x >= left && x <= right && y >= top && y <= bottom) {
      return true;
    }
  }
  return false;
}

// Call this to spawn a bullet from the gun position with velocity based on angle
void spawnBullet(float startX, float startY, float angle) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = startX;
      bullets[i].y = startY;
      float speed = 8.5;  // adjust bullet speed here
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
    for (int j = 0; j < maxEnemies; j++) {
      if (!enemies[j].active) continue;

      int dx = bullets[i].x - enemies[j].x;
      int dy = bullets[i].y - enemies[j].y;
      int distSq = dx * dx + dy * dy;

      if (distSq < 6 * 6) {  // adjust radius for hitbox
        enemies[j].health -= 1;
        bullets[i].active = false;

        if (enemies[j].health <= 0) {
          enemies[j].active = false;
        }
        break; // bullet can only hit one enemy
      }
    }

    if (bullets[i].active) {
      float nextX = bullets[i].x + bullets[i].vx;
      float nextY = bullets[i].y + bullets[i].vy;

      if (isInsideWall((int)nextX, (int)nextY)) {
        bullets[i].active = false;
        continue;
      }

      bullets[i].x = nextX;
      bullets[i].y = nextY;

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

void startReload() {
  if (!isReloading && playerAmmo < maxAmmo) {
    isReloading = true;
    reloadStartTime = millis();
  }
}

void updateReload() {
  if (isReloading && millis() - reloadStartTime >= reloadDuration) {
    playerAmmo = maxAmmo;
    isReloading = false;
  }
}

float getAngleBetweenPoints(int x1, int y1, int x2, int y2) {
  int dx = x2 - x1;
  int dy = y1 - y2;  // Flip Y for screen coordinates
  return atan2((float)dy, (float)dx);
}

void spawnEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    enemies[i].x = random(10, 110);
    enemies[i].y = 100;
    enemies[i].health = 3;   // You can change this to make them tankier
    enemies[i].active = true;

    // Optional: prevent spawning inside a wall
    while (isInsideWall(enemies[i].x, enemies[i].y)) {
      enemies[i].x = random(10, 110);
      enemies[i].y = random(10, 110);
    }
  }
}

void updateEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    if (!enemies[i].active) continue;

    float angleToPlayer = getAngleBetweenPoints(enemies[i].x, enemies[i].y, playerX, playerY);
    
    int nextX = enemies[i].x + enemySpeed * cos(angleToPlayer);
    int nextY = enemies[i].y - enemySpeed * sin(angleToPlayer);

    if (!isInsideWall(nextX, nextY)) {
      enemies[i].x = nextX;
      enemies[i].y = nextY;
    }
  }
}


void drawEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    if (!enemies[i].active) continue;
    display.drawCircle(enemies[i].x, enemies[i].y, 4, SH110X_WHITE);
  }
}


void veridium() {  //launch veridium
  spawnEnemies();
  while (!stopVeridium) {

    updateGyro();
    cursorX = angleX;
    cursorY = angleY;

    updateButtonStates();

    float angle = getAngleBetweenPoints(playerX, playerY, angleX, angleY);

    if (rightButtonState) {
      int nextX = playerX + playerSpeed * cos(angle);
      int nextY = playerY - playerSpeed * sin(angle);
      if (!isInsideWall(nextX, nextY)) {
        playerX = nextX;
        playerY = nextY;
        angleX = playerX;
        angleY = playerY;
        angleX += playerSpeed * 2 * cos(angle);
        angleY -= playerSpeed * 2 * sin(angle);
      }
    }

    if (leftButtonState) {
      angleX = 64;
      angleY = 64;
    }

    if (middleButtonState && playerAmmo > 0) {
      spawnBullet(gunXB, gunYB, angle);
      playerAmmo -= 1;
    }

    if (playerAmmo < 1) {
      startReload();
    }

    updateReload();

    gunXA = playerX;
    gunYA = playerY;

    gunXA += 8 * cos(angle);
    gunYA -= 8 * sin(angle);

    gunXB = gunXA;
    gunYB = gunYA;

    gunXB += 4 * cos(angle);
    gunYB -= 4 * sin(angle);

    updateBullets();

    updateEnemies();

    display.clearDisplay();
    drawWalls();
    // draw player and gun as before
    display.drawCircle(playerX, playerY, 4, SH110X_WHITE);
    display.drawLine(gunXA, gunYA, gunXB, gunYB, SH110X_WHITE);
    drawEnemies();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(gunXB + -30 * cos(angle), gunYB - -30 * sin(angle));
    if (isReloading) {
      display.print("R");
    } else {
      display.print(playerAmmo);
    }

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

    for (int i = 0; i < maxEnemies; i++) {
      if (!enemies[i].active) continue;

      int dx = playerX - enemies[i].x;
      int dy = playerY - enemies[i].y;
      int distanceSquared = dx * dx + dy * dy;

      if (distanceSquared < 8 * 8) {
        display.clearDisplay();
        display.setCursor(20, 60);
        display.print("GAME OVER");
        display.display();
        delay(1000);
        stopVeridium = true;
        return;
      }
    }

    display.display();
    delay(10);

  }
  stopVeridium = false;

}