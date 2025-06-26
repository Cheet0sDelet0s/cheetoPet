#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "flappybird.h" 

DRAM_ATTR float birdY = 32;
DRAM_ATTR float birdVY = 0;
DRAM_ATTR bool alreadyJumped = false;
DRAM_ATTR int pipeTimer = 0;
DRAM_ATTR int flappyScore = 0;

const int maxPipes = 2;
const int pipeGap = 40;
const int pipeWidth = 10;
const int screenW = 128;
const int screenH = 128;

struct Pipe {
  int x;
  int y; // Top of gap
  bool active;
};

Pipe pipes[maxPipes];

void spawnPipe() {
  for (int i = 0; i < maxPipes; i++) {
    if (!pipes[i].active) {
      flappyScore += 1;
      pipes[i].x = screenW;
      pipes[i].y = random(0, screenH - pipeGap);
      pipes[i].active = true;
      break;
    }
  }
}

void updatePipes() {
  for (int i = 0; i < maxPipes; i++) {
    if (pipes[i].active) {
      pipes[i].x -= 2;
      if (pipes[i].x + pipeWidth < 0) {
        pipes[i].active = false;
      }
    }
  }
}

bool checkCollision() {
  if (birdY < 0 || birdY > screenH) return true;

  for (int i = 0; i < maxPipes; i++) {
    if (!pipes[i].active) continue;
    if (10 + 3 >= pipes[i].x && 10 - 3 <= pipes[i].x + pipeWidth) {
      if (birdY - 3 < pipes[i].y || birdY + 3 > pipes[i].y + pipeGap) {
        return true;
      }
    }
  }

  return false;
}

void drawPipes() {
  for (int i = 0; i < maxPipes; i++) {
    if (pipes[i].active) {
      // Top pipe
      display.fillRect(pipes[i].x, 0, pipeWidth, pipes[i].y, SH110X_WHITE);
      // Bottom pipe
      display.fillRect(pipes[i].x, pipes[i].y + pipeGap, pipeWidth, screenH - (pipes[i].y + pipeGap), SH110X_WHITE);
    }
  }
}

void flappyBird() {
  birdY = 32;
  birdVY = 0;
  alreadyJumped = false;
  pipeTimer = 0;
  flappyScore = 0;

  // Reset pipes
  for (int i = 0; i < maxPipes; i++) {
    pipes[i].active = false;
  }

  while (1) {
    updateButtonStates();

    if (rightButtonState && !alreadyJumped) {
      birdVY = -3.5;
      alreadyJumped = true;
    } else if (!rightButtonState) {
      alreadyJumped = false;
    }

    birdVY += 0.25;
    birdY += birdVY;
    pipeTimer++;

    if (pipeTimer > 60) {
      pipeTimer = 0;
      spawnPipe();
    }

    updatePipes();

    display.clearDisplay();
    drawPipes();
    display.fillCircle(10, birdY, 3, SH110X_WHITE);
    display.drawLine(10, birdY, 16, birdY + birdVY, SH110X_WHITE);
    display.setTextColor(SH110X_WHITE, SH110X_BLACK);
    display.setCursor(55, 0);
    display.print(flappyScore);
    display.display();

    if (checkCollision()) {
      // Simple game over
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(20, 30);
      display.setTextColor(SH110X_WHITE);
      display.println("Game Over!");
      display.display();
      delay(1000);
      break;
    }

    if (leftButtonState) {
      break; // Exit the game loop manually
    }

    delay(20);
  }
  petFun += constrain(flappyScore, 0, 20);
  money += flappyScore / 2;

  petMessage("ahh ggs ggs");
}
