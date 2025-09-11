#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "veridium.h"

#define MAX_BULLETS 50  // max bullets on screen
#define MAX_ENEMIES 10  // Maximum number of enemies

DRAM_ATTR float petVeridiumXP = 0;
DRAM_ATTR int petVeridiumLVL = 1;
DRAM_ATTR bool stopVeridium = false;

DRAM_ATTR int playerX = 64;
DRAM_ATTR int playerY = 64;
DRAM_ATTR int gunXA = 0;
DRAM_ATTR int gunYA = 0;
DRAM_ATTR int gunXB = 0;
DRAM_ATTR int gunYB = 0;
DRAM_ATTR int playerAmmo = 50;

int veridiumScore = 0;

const int maxAmmo = 200;

int enemySpawnTime = 1000;
unsigned long lastEnemySpawnTime = 0;

DRAM_ATTR bool isReloading = false;
DRAM_ATTR unsigned long reloadStartTime = 0;
DRAM_ATTR const unsigned long reloadDuration = 1000;


struct Bullet {
  float x, y;
  float vx, vy;
  bool active;
};

Bullet bullets[MAX_BULLETS];

struct Enemy {
  float x, y;
  float vx, vy;
  bool active;
};

Enemy enemies[MAX_ENEMIES];

// Call this to spawn a bullet from the gun position with velocity based on angle
void spawnBullet(float startX, float startY, float angle) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      angle += random(-10, 10) / 100.0;  // add slight inaccuracy
      bullets[i].x = startX;
      bullets[i].y = startY;
      float speed = 8.5;  // adjust bullet speed here
      bullets[i].vx = speed * cos(angle);
      bullets[i].vy = -speed * sin(angle);  // minus because screen Y-axis flips
      bullets[i].active = true;
      queueTone(random(100, 150), 1);
      break;
    }
  }
}

// Update all bullets position and deactivate if off screen
void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {

    if (bullets[i].active) {
      float nextX = bullets[i].x + bullets[i].vx;
      float nextY = bullets[i].y + bullets[i].vy;

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

// Initialize enemies
void spawnEnemy(float startX, float startY) {
  const float minSpawnDistance = 64.0;  // Minimum distance from the player
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      float dx = playerX - startX;
      float dy = playerY - startY;
      float distance = sqrt(dx * dx + dy * dy);

      // If the enemy is too close, move it further away
      if (distance < minSpawnDistance) {
        float angle = atan2(dy, dx);
        startX = playerX - minSpawnDistance * cos(angle);
        startY = playerY - minSpawnDistance * sin(angle);
      }

      enemies[i].x = startX;
      enemies[i].y = startY;
      enemies[i].active = true;
      queueTone(200, 20);
      break;
    }
  }
}

// Update enemy positions and check for collisions with bullets
void updateEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      // Move enemy toward the player
      float dx = playerX - enemies[i].x;
      float dy = playerY - enemies[i].y;
      float distance = sqrt(dx * dx + dy * dy);
      float speed = 1.0;  // Adjust enemy speed here
      enemies[i].vx = speed * (dx / distance);
      enemies[i].vy = speed * (dy / distance);

      enemies[i].x += enemies[i].vx;
      enemies[i].y += enemies[i].vy;

      // Check for collisions with bullets
      for (int j = 0; j < MAX_BULLETS; j++) {
        if (bullets[j].active) {
          float bulletDx = bullets[j].x - enemies[i].x;
          float bulletDy = bullets[j].y - enemies[i].y;
          float bulletDistance = sqrt(bulletDx * bulletDx + bulletDy * bulletDy);

          if (bulletDistance < 5) {  // Collision threshold
            bullets[j].active = false;
            enemies[i].active = false;
            veridiumScore++;
            queueTone(800, 30);
            break;
          }
        }
      }

      // Deactivate enemy if it reaches the player
      if (distance < 4) {
        enemies[i].active = false;

        stopVeridium = true;
      }
    }
  }
}

// Draw all active enemies on the screen
void drawEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      display.drawCircle((int)enemies[i].x, (int)enemies[i].y, 4, SH110X_WHITE);
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
  // Ensure correct angle calculation considering the screen's coordinate system
  int dx = x2 - x1;
  int dy = y1 - y2;  // Flip Y-axis for screen coordinates
  return atan2((float)dy, (float)dx);
}

void veridium() {  //launch veridium
  veridiumScore = 0;
  clearTones();
  playerAmmo = maxAmmo;

  for (int j = 0; j < MAX_BULLETS; j++) {
    enemies[j].active = false;
  }

  while (!stopVeridium) {

    updateGyro();
    cursorX = angleX;
    cursorY = angleY;

    updateButtonStates();

    float angle = getAngleBetweenPoints(playerX, playerY, angleX, angleY);

    if (middleButtonState) {
      stopVeridium = true;
    }

    if (leftButtonState) {
      angleX = 64;
      angleY = 64;
    }

    if (rightButtonState && playerAmmo > 0) {
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
    updateEnemies();  // Update enemies

    display.clearDisplay();
    // draw player and gun as before
    display.drawCircle(playerX, playerY, 4, SH110X_WHITE);
    display.drawLine(gunXA, gunYA, gunXB, gunYB, SH110X_WHITE);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE, SH110X_BLACK);
    display.setCursor(gunXB + -30 * cos(angle), gunYB - -30 * sin(angle));
    if (isReloading) {
      display.print("R");
    } else {
      display.print(playerAmmo);
    }
    display.setCursor(0, 0);
    display.print(veridiumScore);

    //display.fillCircle(angleX, angleY, 2, SH110X_WHITE);

    display.drawFastHLine(angleX - 2, angleY, 5, SH110X_WHITE);
    display.drawFastVLine(angleX, angleY - 2, 5, SH110X_WHITE);

    // draw bullets
    drawBullets();

    // draw enemies
    drawEnemies();

    display.display();
    audioEngine();
    delay(10);

    // Spawn enemies periodically
    if (enemySpawnTime > 0 && millis() - lastEnemySpawnTime >= enemySpawnTime) {
      lastEnemySpawnTime = millis();
      spawnEnemy(random(0, 128), random(0, 128));
      enemySpawnTime = random(200, 800);
    }
  }
  stopVeridium = false;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(15, 20);
  display.println("game over!");
  display.setCursor(15, 50);
  display.print("you got "); display.print(veridiumScore); display.print(" points");
  display.setCursor(15, 60);
  display.print("press A to exit");
  display.display();

  waitForSelectRelease();
  updateButtonStates();
  while (!rightButtonState) {updateButtonStates(); delay(20);}
}