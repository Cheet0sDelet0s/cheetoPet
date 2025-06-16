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

float petVeridiumXP = 0;
int petVeridiumLVL = 1;
bool stopVeridium = false;

int playerX = 10;
int playerY = 10;
int gunXA = 0;
int gunYA = 0;
int gunXB = 0;
int gunYB = 0;
int playerSpeed = 2;

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

    gunXA = playerX;
    gunYA = playerY;

    gunXA += 8 * cos(angle);
    gunYA -= 8 * sin(angle);

    gunXB = gunXA;
    gunYB = gunYA;

    gunXB += 4 * cos(angle);
    gunYB -= 4 * sin(angle);

    display.clearDisplay();
    display.drawCircle(playerX, playerY, 4, SH110X_WHITE);
    display.drawLine(gunXA, gunYA, gunXB, gunYB, SH110X_WHITE);
    display.fillCircle(angleX, angleY, 2, SH110X_WHITE);
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