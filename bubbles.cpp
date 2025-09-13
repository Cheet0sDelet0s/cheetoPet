#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "bubbles.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define NUM_PARTICLES 175
#define PARTICLE_RADIUS 2
#define CELL_SIZE 8
#define GRID_COLS (SCREEN_WIDTH / CELL_SIZE)
#define GRID_ROWS (SCREEN_HEIGHT / CELL_SIZE)
#define MAX_PARTICLES_PER_CELL 8

Bubble bubbles[300];
uint8_t grid[GRID_COLS][GRID_ROWS][MAX_PARTICLES_PER_CELL];
uint8_t gridCounts[GRID_COLS][GRID_ROWS];

int8_t jitterLUT[16] = { -5, -3, -1, 0, 1, 3, 5, -4, 2, -2, 4, -1, 1, 0, -3, 3 };

int bubbleCount = 0;

bool zeroG = false;

void initBubbles() {
  bubbleCount = 0;
  
  for (int i = 0; i < NUM_PARTICLES; i++) {
    bubbles[i].x = random(SCREEN_WIDTH);
    bubbles[i].y = random(SCREEN_HEIGHT);
    bubbles[i].vx = 0;
    bubbles[i].vy = 0;
    bubbleCount++;
  }
}

void spawnBubble() {
  bubbles[bubbleCount].x = random(SCREEN_WIDTH);
  bubbles[bubbleCount].y = random(SCREEN_HEIGHT);
  bubbles[bubbleCount].vx = 0;
  bubbles[bubbleCount].vy = 0;
  bubbleCount++;
}

void clearGrid() {
  for (int x = 0; x < GRID_COLS; x++) {
    for (int y = 0; y < GRID_ROWS; y++) {
      gridCounts[x][y] = 0;
    }
  }
}

void populateGrid() {
  for (int i = 0; i < bubbleCount; i++) {
    int gx = constrain((int)(bubbles[i].x) / CELL_SIZE, 0, GRID_COLS - 1);
    int gy = constrain((int)(bubbles[i].y) / CELL_SIZE, 0, GRID_ROWS - 1);
    if (gridCounts[gx][gy] < bubbleCount) {
      grid[gx][gy][gridCounts[gx][gy]++] = i;
    }
  }
}

void updateBubbles() {
  float ax = angleX * 0.05; // Tweak this for responsiveness
  float ay = angleY * 0.05;

  clearGrid();
  populateGrid();

  for (int i = 0; i < bubbleCount; i++) {
    // Add a slight bit of randomness
    int index = i % 16;
    float jitterX = jitterLUT[index] / 100.0;
    float jitterY = jitterLUT[(index + 5) % 16] / 100.0;

    bubbles[i].vx += ax + jitterX;
    bubbles[i].vy += ay + jitterY;

    bubbles[i].vx = constrain(bubbles[i].vx, -4, 4);
    bubbles[i].vy = constrain(bubbles[i].vy, -4, 4);

    bubbles[i].x += bubbles[i].vx;
    bubbles[i].y += bubbles[i].vy;

    int gx = constrain((int)(bubbles[i].x) / CELL_SIZE, 0, GRID_COLS - 1);
    int gy = constrain((int)(bubbles[i].y) / CELL_SIZE, 0, GRID_ROWS - 1);

    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        int nx = gx + dx;
        int ny = gy + dy;
        if (nx < 0 || ny < 0 || nx >= GRID_COLS || ny >= GRID_ROWS) continue;
        for (int k = 0; k < gridCounts[nx][ny]; k++) {
          int j = grid[nx][ny][k];
          if (i == j) continue;

          float dx = bubbles[j].x - bubbles[i].x;
          float dy = bubbles[j].y - bubbles[i].y;
          float distSq = dx * dx + dy * dy;
          float minDist = 2 * PARTICLE_RADIUS;

          if (distSq < minDist * minDist) {
            float dist = sqrt(distSq);
            float overlap = 0.5 * (minDist - dist);

            if (dist != 0) {
              dx /= dist;
              dy /= dist;
            } else {
              dx = 1;
              dy = 0;
            }

            bubbles[i].x -= dx * overlap;
            bubbles[i].y -= dy * overlap;
            bubbles[j].x += dx * overlap;
            bubbles[j].y += dy * overlap;

            bubbles[i].vx *= 0.8;
            bubbles[i].vy *= 0.8;

            // have a change to generate a tone when a collision happens
            if (random(0, 1000) == 1) { queueTone(random(180, 220), 1); }
          }
        }
      }
    }

    if (bubbles[i].x < 0) {
      bubbles[i].x = 0;
      bubbles[i].vx *= -0.6;
    }
    if (bubbles[i].x >= SCREEN_WIDTH) {
      bubbles[i].x = SCREEN_WIDTH - 1;
      bubbles[i].vx *= -0.6;
    }
    if (bubbles[i].y < 0) {
      bubbles[i].y = 0;
      bubbles[i].vy *= -0.6;
    }
    if (bubbles[i].y >= SCREEN_HEIGHT) {
      bubbles[i].y = SCREEN_HEIGHT - 1;
      bubbles[i].vy *= -0.6;
    }
  }
}

void drawBubbles() {
  display.clearDisplay();
  for (int i = 0; i < bubbleCount; i++) {
    //display.drawPixel((int)bubbles[i].x, (int)bubbles[i].y, SH110X_WHITE);
    display.fillCircle((int)bubbles[i].x, (int)bubbles[i].y, PARTICLE_RADIUS + 1, SH110X_BLACK);
    display.drawCircle((int)bubbles[i].x, (int)bubbles[i].y, PARTICLE_RADIUS, SH110X_WHITE);
  }
  if (leftButtonState) {
    display.setCursor(0, 0);
    display.setTextColor(SH110X_WHITE, SH110X_BLACK);
    if (!zeroG) {
      display.print("zero g on");  
    } else {
      display.print("zero g off");
    }
  }
  display.display();
  screenRecord();
}

void bubbleSim() {
  initBubbles();
  updateButtonStates();
  clearTones();
  while (!rightButtonState) {
    updateGyro();
    updateBubbles();
    updateButtonStates();
    drawBubbles();
    if (!leftButtonState && previousLeftState) {
      zeroG = !zeroG;
    }

    if (!middleButtonState && previousMiddleState) {
      spawnBubble();
    }

    if (zeroG) {
      angleX = 0;
      angleY = 0;
    } else {
      angleX = constrain(angleX, -4, 4);
      angleY = constrain(angleY, -4, 4);
    }
    
    previousLeftState = leftButtonState;
    previousMiddleState = middleButtonState;
    audioEngine();
    delay(5); // Limit frame rate
  }
}
