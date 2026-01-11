#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "particlesystem.h"

#define MAX_PARTICLES 50

Particle particles[MAX_PARTICLES];
int particleCount = 0;

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