#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "particlesim.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define NUM_PARTICLES 100
#define PARTICLE_RADIUS 3

Particle particles[NUM_PARTICLES];

void initParticles() {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].x = random(SCREEN_WIDTH);
    particles[i].y = random(SCREEN_HEIGHT);
    particles[i].vx = 0;
    particles[i].vy = 0;
  }
}

void updateParticles() {
  float ax = angleX * 0.05; // Tweak this for responsiveness
  float ay = angleY * 0.05;

  for (int i = 0; i < NUM_PARTICLES; i++) {
    // Add a slight bit of randomness
    float jitterX = ((float)random(-5, 6)) / 100.0;
    float jitterY = ((float)random(-5, 6)) / 100.0;

    particles[i].vx += ax + jitterX;
    particles[i].vy += ay + jitterY;

    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;

    // Simple particle collision detection and stacking
    for (int j = 0; j < NUM_PARTICLES; j++) {
      if (i == j) continue;
      float dx = particles[j].x - particles[i].x;
      float dy = particles[j].y - particles[i].y;
      float distSq = dx * dx + dy * dy;
      float minDist = 2 * PARTICLE_RADIUS;

      if (distSq < minDist * minDist) {
        float dist = sqrt(distSq);
        float overlap = 0.5 * (minDist - dist);

        // Normalize the direction vector
        if (dist != 0) {
          dx /= dist;
          dy /= dist;
        } else {
          dx = 1;
          dy = 0;
        }

        // Push particles apart
        particles[i].x -= dx * overlap;
        particles[i].y -= dy * overlap;
        particles[j].x += dx * overlap;
        particles[j].y += dy * overlap;

        // Exchange velocities
        float tempVx = particles[i].vx;
        float tempVy = particles[i].vy;
        particles[i].vx = particles[j].vx;
        particles[i].vy = particles[j].vy;
        particles[j].vx = tempVx;
        particles[j].vy = tempVy;
      }
    }

    // Bounce off edges
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
  for (int i = 0; i < NUM_PARTICLES; i++) {
    display.drawPixel((int)particles[i].x, (int)particles[i].y, SH110X_WHITE);
  }
  display.display();
}

void particleSim() {
  initParticles();
  updateButtonStates();
  while (!rightButtonState) {
    updateGyro();
    updateParticles();
    updateButtonStates();
    drawParticles();
    if (leftButtonState) {
      angleX = 0;
      angleY = 0;
    }
    delay(5); // Limit frame rate
  }
}
