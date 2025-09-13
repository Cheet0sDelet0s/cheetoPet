#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <Arduino.h>

extern Adafruit_SH1107 display;

struct Particle {
  int type, lifetime, maxlife;
  float x, y;
  float vx, vy;
};

void updateParticles();
void drawParticles();
void createParticle(int type, float x, float y, float vx, float vy, int lifetime);

#endif