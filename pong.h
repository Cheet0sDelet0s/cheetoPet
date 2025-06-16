#ifndef PONG_H
#define PONG_H

#include "bitmaps.h"

// DRAM_ATTR int score;
// DRAM_ATTR int leftPaddlePos;
// DRAM_ATTR float ballX;
// DRAM_ATTR float ballY;
// DRAM_ATTR float ballVX;
// DRAM_ATTR float ballVY;
// DRAM_ATTR int ballSpeed;
// DRAM_ATTR int paddleX;
// DRAM_ATTR int paddleY;
// DRAM_ATTR int paddleHeight;
// DRAM_ATTR bool stopApp;
// DRAM_ATTR float friction;
// DRAM_ATTR float minHorizontalSpeed;
// DRAM_ATTR float maxVerticalSpeed;
// DRAM_ATTR float enemyX;          // Opposite side of player
// DRAM_ATTR float enemyY;           // Starting Y
// DRAM_ATTR int enemyHeight;
// DRAM_ATTR float enemySpeed;      // How fast the enemy moves (tweakable)
// DRAM_ATTR int enemyScore;
extern float petPongXP;
extern int petPongLVL;

extern Adafruit_SH1107 display;

extern float angleY;
extern void petMessage(String message);
extern float money;
extern int petFun;
extern bool leftButtonState;
extern bool rightButtonState;
extern void updateButtonStates();
extern void updateGyro();
extern void drawCheckerboard(uint8_t squareSize);
extern void drawCenteredText(Adafruit_GFX &display, const String &text, int16_t y);
//extern const BitmapInfo bitmaps;

void drawPetLeveling(String levelType, float beginningXP, float gainedXP, int beginningLVL);

void stepBallForward();

void reEnergizeBall(float amount);

void pong();

#endif