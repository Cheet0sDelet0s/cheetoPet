#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "particlesim.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define NUM_PARTICLES 175
#define PARTICLE_RADIUS 2
#define CELL_SIZE 8
#define GRID_COLS (SCREEN_WIDTH / CELL_SIZE)
#define GRID_ROWS (SCREEN_HEIGHT / CELL_SIZE)
#define MAX_PARTICLES_PER_CELL 8

Particle particles[300];
uint8_t grid[GRID_COLS][GRID_ROWS][MAX_PARTICLES_PER_CELL];
uint8_t gridCounts[GRID_COLS][GRID_ROWS];

int8_t jitterLUT[16] = { -5, -3, -1, 0, 1, 3, 5, -4, 2, -2, 4, -1, 1, 0, -3, 3 };

int particleCount = 0;

bool previousLeftState = false;
bool previousMiddleState = false;

bool zeroG = false;

void initParticles() {
  particleCount = 0;
  
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].x = random(SCREEN_WIDTH);
    particles[i].y = random(SCREEN_HEIGHT);
    particles[i].vx = 0;
    particles[i].vy = 0;
    particleCount++;
  }
}

void spawnParticle() {
  particles[particleCount].x = random(SCREEN_WIDTH);
  particles[particleCount].y = random(SCREEN_HEIGHT);
  particles[particleCount].vx = 0;
  particles[particleCount].vy = 0;
  particleCount++;
}

void clearGrid() {
  for (int x = 0; x < GRID_COLS; x++) {
    for (int y = 0; y < GRID_ROWS; y++) {
      gridCounts[x][y] = 0;
    }
  }
}

void populateGrid() {
  for (int i = 0; i < particleCount; i++) {
    int gx = constrain((int)(particles[i].x) / CELL_SIZE, 0, GRID_COLS - 1);
    int gy = constrain((int)(particles[i].y) / CELL_SIZE, 0, GRID_ROWS - 1);
    if (gridCounts[gx][gy] < particleCount) {
      grid[gx][gy][gridCounts[gx][gy]++] = i;
    }
  }
}

void updateParticles() {
  float ax = angleX * 0.05; // Tweak this for responsiveness
  float ay = angleY * 0.05;

  clearGrid();
  populateGrid();

  for (int i = 0; i < particleCount; i++) {
    // Add a slight bit of randomness
    int index = i % 16;
    float jitterX = jitterLUT[index] / 100.0;
    float jitterY = jitterLUT[(index + 5) % 16] / 100.0;

    particles[i].vx += ax + jitterX;
    particles[i].vy += ay + jitterY;

    particles[i].vx = constrain(particles[i].vx, -4, 4);
    particles[i].vy = constrain(particles[i].vy, -4, 4);

    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;

    int gx = constrain((int)(particles[i].x) / CELL_SIZE, 0, GRID_COLS - 1);
    int gy = constrain((int)(particles[i].y) / CELL_SIZE, 0, GRID_ROWS - 1);

    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        int nx = gx + dx;
        int ny = gy + dy;
        if (nx < 0 || ny < 0 || nx >= GRID_COLS || ny >= GRID_ROWS) continue;
        for (int k = 0; k < gridCounts[nx][ny]; k++) {
          int j = grid[nx][ny][k];
          if (i == j) continue;

          float dx = particles[j].x - particles[i].x;
          float dy = particles[j].y - particles[i].y;
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

            particles[i].x -= dx * overlap;
            particles[i].y -= dy * overlap;
            particles[j].x += dx * overlap;
            particles[j].y += dy * overlap;

            particles[i].vx *= 0.8;
            particles[i].vy *= 0.8;
          }
        }
      }
    }

    if (particles[i].x < 0) {
      particles[i].x = 0;
      particles[i].vx *= -0.6;
    }
    if (particles[i].x >= SCREEN_WIDTH) {
      particles[i].x = SCREEN_WIDTH - 1;
      particles[i].vx *= -0.6;
    }
    if (particles[i].y < 0) {
      particles[i].y = 0;
      particles[i].vy *= -0.6;
    }
    if (particles[i].y >= SCREEN_HEIGHT) {
      particles[i].y = SCREEN_HEIGHT - 1;
      particles[i].vy *= -0.6;
    }
  }
}

void drawParticles() {
  display.clearDisplay();
  for (int i = 0; i < particleCount; i++) {
    //display.drawPixel((int)particles[i].x, (int)particles[i].y, SH110X_WHITE);
    display.fillCircle((int)particles[i].x, (int)particles[i].y, PARTICLE_RADIUS + 1, SH110X_BLACK);
    display.drawCircle((int)particles[i].x, (int)particles[i].y, PARTICLE_RADIUS, SH110X_WHITE);
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

void particleSim() {
  initParticles();
  updateButtonStates();
  while (!rightButtonState) {
    updateGyro();
    updateParticles();
    updateButtonStates();
    drawParticles();
    if (!leftButtonState && previousLeftState) {
      zeroG = !zeroG;
    }

    if (!middleButtonState && previousMiddleState) {
      spawnParticle();
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
    delay(5); // Limit frame rate
  }
}
