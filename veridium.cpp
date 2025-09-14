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

const int maxAmmo = 100;

// Shared corkscrew angle (one value for the whole game)
static float sharedSpiralAngle = 0.0f;
static const float SHARED_SPIRAL_TURN_SPEED = 0.07f; // radians per frame — tweak this

int enemySpawnTime = 1000;
unsigned long lastEnemySpawnTime = 0;

DRAM_ATTR bool isReloading = false;
DRAM_ATTR unsigned long reloadStartTime = 0;
DRAM_ATTR const unsigned long reloadDuration = 1000;

int currentWave = 1;  // Track the current wave
int waveEnemyCount = 5;  // Number of enemies in the current wave
int enemiesSpawnedThisWave = 0;  // Track how many enemies have been spawned in the wave

unsigned long wavePauseStartTime = 0;  // Track the start time of the wave pause
bool isWavePaused = false;  // Flag to indicate if the game is in a wave pause

struct Bullet {
  float x, y;
  float vx, vy;
  bool active;
};

Bullet bullets[MAX_BULLETS];

struct Enemy {
  float x, y;
  float vx, vy;
  int type;
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
  if (enemiesSpawnedThisWave >= waveEnemyCount) {
    return;  // Don't spawn more enemies than allowed for the wave
  }

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
      
      int randomiser = random(0, 14);

      if (randomiser == 0) {
        enemies[i].type = 2;
      // } else if (randomiser < 4) { 
      //   enemies[i].type = 3;
      } else {
        enemies[i].type = 1;
      }
      
      enemies[i].active = true;
      queueTone(200, 20);
      enemiesSpawnedThisWave++;  // Increment the count of spawned enemies
      break;
    }
  }
}

void updateEnemies() {
  // update the shared spiral angle once per frame
  sharedSpiralAngle += SHARED_SPIRAL_TURN_SPEED;
  if (sharedSpiralAngle > 2.0f * 3.14159265f) sharedSpiralAngle -= 2.0f * 3.14159265f;

  int activeEnemies = 0;  // Count active enemies
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;
    activeEnemies++;

    // Vector from enemy to player
    float dx = playerX - enemies[i].x;
    float dy = playerY - enemies[i].y;
    float distance = sqrtf(dx * dx + dy * dy);
    float speed = 1.0f;  // Adjust enemy speed here

    if (distance < 0.001f) {
      // Avoid divide-by-zero; just stop if exactly on player
      enemies[i].vx = 0.0f;
      enemies[i].vy = 0.0f;
    } else {
      if (enemies[i].type == 3) {
        // Corkscrew spiral: rotate the "toward player" angle by the shared offset
        float angleToPlayer = atan2f(dy, dx);
        float angle = angleToPlayer + sharedSpiralAngle;

        // Velocity is a unit vector in that rotated direction times speed
        enemies[i].vx = speed * cosf(angle);
        enemies[i].vy = speed * sinf(angle);
      } else {
        // Straight movement for other enemy types (normalized)
        enemies[i].vx = speed * (dx / distance);
        enemies[i].vy = speed * (dy / distance);
      }
    }

    enemies[i].x += enemies[i].vx;
    enemies[i].y += enemies[i].vy;

    // Check for collisions with bullets
    for (int j = 0; j < MAX_BULLETS; j++) {
      if (!bullets[j].active) continue;
      float bulletDx = bullets[j].x - enemies[i].x;
      float bulletDy = bullets[j].y - enemies[i].y;
      float bulletDistance = sqrtf(bulletDx * bulletDx + bulletDy * bulletDy);

      if (bulletDistance < 5.0f) {  // Collision threshold
        bullets[j].active = false;
        enemies[i].active = false;
        if (enemies[i].type == 2) {
          playerAmmo += 50;
          queueTone(1000, 30);
        }
        veridiumScore++;
        queueTone(800, 30);
        break;
      }
    }

    // Deactivate enemy if it reaches the player
    if (distance < 4.0f) {
      enemies[i].active = false;
      stopVeridium = true;
    }
  }

  // Check if the wave is complete
  if (activeEnemies == 0 && enemiesSpawnedThisWave >= waveEnemyCount) {
    if (!isWavePaused) {
      isWavePaused = true;
      wavePauseStartTime = millis();  // Start the wave pause timer
    } else if (millis() - wavePauseStartTime >= 1500) {  // check if time has passed
      isWavePaused = false;  // End the wave pause
      currentWave++;  // Advance to the next wave
      waveEnemyCount += 5;  // Increase the number of enemies for the next wave
      enemiesSpawnedThisWave = 0;  // Reset the spawn count for the new wave
      enemySpawnTime = max(200, enemySpawnTime - 50);  // Optionally decrease spawn time
    }
  }
}
// Draw all active enemies on the screen
void drawEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      display.drawCircle((int)enemies[i].x, (int)enemies[i].y, 4, SH110X_WHITE);
      if (enemies[i].type == 2) {
        display.setCursor((int)enemies[i].x - 3, (int)enemies[i].y - 3);
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.print("+");
      }
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

  currentWave = 1;  // Reset wave to 1 at the start of the game
  waveEnemyCount = 5;  // Reset initial wave enemy count
  enemiesSpawnedThisWave = 0;  // Reset spawned enemies count
  enemySpawnTime = 1000;  // Reset spawn time

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

    static unsigned long lastFireTime = 0;
    const unsigned long fireRateDelay = 60;  // Fire rate delay in milliseconds

    if (rightButtonState && playerAmmo > 0 && millis() - lastFireTime >= fireRateDelay) {
      spawnBullet(gunXB, gunYB, angle);
      playerAmmo -= 1;
      lastFireTime = millis();
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

    display.setCursor(0, 120);
    display.print("Wave: ");
    display.print(currentWave);  // Display the current wave

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

  petFun += veridiumScore / 5;
  if (petFun > 100) petFun = 100;

  waitForSelectRelease();
  updateButtonStates();
  while (!rightButtonState) {updateButtonStates(); delay(20);}
}