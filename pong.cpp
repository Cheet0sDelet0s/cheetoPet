#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "pong.h"
#include "particlesystem.h"

DRAM_ATTR int score = 0;
DRAM_ATTR int leftPaddlePos = 0;
DRAM_ATTR float ballX = 10;
DRAM_ATTR float ballY = 30;
DRAM_ATTR float ballVX = 2;
DRAM_ATTR float ballVY = 2;
DRAM_ATTR int ballSpeed = 5;
DRAM_ATTR int paddleX = 0;
DRAM_ATTR int paddleY = 0;
DRAM_ATTR int paddleHeight = 20;
DRAM_ATTR bool stopApp = false;
DRAM_ATTR float friction = 0.998;
DRAM_ATTR float minHorizontalSpeed = 0.8;
DRAM_ATTR float maxVerticalSpeed = 3;
DRAM_ATTR float enemyPaddleX = 125;          // Opposite side of player
DRAM_ATTR float enemyPaddleY = 30;           // Starting Y
DRAM_ATTR int enemyHeight = 20;
DRAM_ATTR float enemySpeed = 1.5;      // How fast the enemy moves (tweakable)
DRAM_ATTR int enemyScore = 0;
DRAM_ATTR float petPongXP = 0;
DRAM_ATTR int petPongLVL = 1;

void handlePongEnemyAI() {
  if (ballX > 64) {
    int difference = (ballY + 1) - (enemyPaddleY + enemyHeight / 2); // Adjust for ball center
    enemyPaddleY += difference / 9 - petPongLVL / 2;
  
    enemyPaddleY = constrain(enemyPaddleY, 0, 128 - enemyHeight);
  }
}

void drawPetLeveling(String levelType, float beginningXP, float gainedXP, int beginningLVL) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);
  display.print("pet "); display.print(levelType); display.print("\nlvl");
  
  const BitmapInfo& bmp = bitmaps[1];
  display.drawBitmap(50, 20, bmp.data, bmp.width, bmp.height, SH110X_WHITE);
  display.setCursor(70, 26);
  display.setTextSize(1);
  display.print("lvl ");
  float newXP;
  int newLVL;
  if (beginningXP + gainedXP > beginningLVL * 5) {
    display.print(beginningLVL + 1);
    newXP = (beginningXP + gainedXP) - beginningLVL * 5;
    newLVL = beginningLVL + 1;
  } else {
    display.print(beginningLVL);
    newXP = beginningXP + gainedXP;
    newLVL = beginningLVL;
  }

  float xpPercentage = newXP / (newLVL * 5);

  display.drawRect(20, 55, 88, 10, SH110X_WHITE);
  display.fillRect(20, 55, 88 * xpPercentage, 10, SH110X_WHITE);
  
  drawCenteredText(String((String(newXP) + "/" + String(newLVL * 5))), 70);
  drawCenteredText("press A", 118);
  
  display.display();
  updateButtonStates();
  while (!rightButtonState) {updateButtonStates(); delay(20);}
  
  drawCheckerboard(newLVL + 1);
}

void stepBallForward() {
  ballX += ballVX * constrain(enemySpeed / 2, 1, 2);
  ballY += ballVY * constrain(enemySpeed / 2, 1, 2);
}

void reEnergizeBall(float amount) {
  ballVX *= amount + petPongLVL / 10;
  ballVY *= amount + petPongLVL / 10;
}

void pong() { // PONG if you couldnt read
  display.setTextColor(SH110X_WHITE);
  clearTones();
  while (stopApp == false) {
      updateGyro();
      updateButtonStates();
      if (rightButtonState) {
        stopApp = true;
      }
      if (leftButtonState) {
        angleY = 60;
      }
      constrain(angleY + 64, 0, 126);

      paddleY = angleY;

      stepBallForward();
    
      if (ballX <= 0) {
        ballVX *= -1;
        // reEnergizeBall(1.2);
        enemyScore += 1;
        queueTone(200, 100);
        ballX = 40;
      }

      if (ballX >= 128) {
        ballVX *= -1;
        // reEnergizeBall(1.2);
        score += 1;
        queueTone(1000, 100);
        ballX = 100;
      }

      if (ballY >= 127 || ballY <= 0) {
        ballVY *= -1;
        reEnergizeBall(1.12);
      }

      handlePongEnemyAI();

      if (abs(ballVX) < minHorizontalSpeed) {
        ballVX = (ballVX < 0 ? -1 : 1) * minHorizontalSpeed;
        // Optionally scale VY to keep total speed consistent
        float speed = sqrt(ballVX * ballVX + ballVY * ballVY);
        float desiredSpeed = 2.5; // or your current base speed
        float scale = desiredSpeed / speed;
        ballVX *= scale;
        ballVY *= scale;
      }

      if (ballVY > maxVerticalSpeed) {
        ballVY = ballVY / 1.2;
        ballVX *= 1.5;
      }

      
      // Store last frame's ball position
      float lastBallX = ballX - ballVX;
      float lastBallY = ballY - ballVY;

      // Ball is moving left toward player paddle
      if (ballVX < 0 &&
          lastBallX >= paddleX + 4 &&  // was to the right of paddle
          ballX <= paddleX + 4 &&      // now is to the left of paddle
          ballY >= paddleY - 2 &&
          ballY <= paddleY + paddleHeight + 2) {

        ballVX = abs(ballVX);
        float offset = (ballY - (paddleY + paddleHeight / 2)) / (paddleHeight / 2) + random(-5, 5);
        ballVY += offset * 2;
        reEnergizeBall(1.1);
        queueTone(440, 50);
        for (int i = 0; i < 3; i++) {
          createParticle(1, ballX, ballY, ballVX, ballVY + (random(-10, 10) / 10), 20);
        }
      }

      // Ball is moving right toward enemy paddle
      if (ballVX > 0 &&
          lastBallX <= enemyPaddleX - 2 &&  // was to the left
          ballX >= enemyPaddleX - 2 &&     // now is to the right
          ballY >= enemyPaddleY &&
          ballY <= enemyPaddleY + enemyHeight) {

        ballVX = -abs(ballVX);
        float offset = (ballY - (enemyPaddleY + enemyHeight / 2)) / (enemyHeight / 2);
        ballVY += offset;
        queueTone(440, 50);
        for (int i = 0; i < 3; i++) {
          createParticle(1, ballX, ballY, ballVX / 3, ballVY / 3 + (random(-10, 10) / 10), 20);
        }
      }

      if (ballY <= 0) {
        ballY = 0;
        ballVY = abs(ballVY)*1.5; // Bounce down
        queueTone(420, 50);
      }
      if (ballY >= 127) {
        ballY = 127;
        ballVY = -abs(ballVY)*1.5; // Bounce up
        queueTone(420, 50);
      }

      
      float speed = sqrt(ballVX * ballVX + ballVY * ballVY);
      
      if (speed > 4) {
        ballVX *= friction / 1.5;
        ballVY *= friction / 1.5;
      } else if (speed > 3) {
        ballVX *= friction;
        ballVY *= friction;
      } else {
        reEnergizeBall(2);
      }
      

      display.clearDisplay();
      display.fillCircle(ballX, ballY, 2, SH110X_WHITE);
      display.fillRect(paddleX, paddleY, 2, paddleHeight, SH110X_WHITE);
      display.fillRect(enemyPaddleX, enemyPaddleY, 2, enemyHeight, SH110X_WHITE);
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.print(score);
      display.setCursor(120, 0);
      display.print(enemyScore);

      updateParticles();
      drawParticles();

      display.display();
      screenRecord();
      audioEngine();
      if (enemyScore == 5 || score == 5) {
        stopApp = true;
      }
      delay(20);
  }
  stopApp = false;
  petFun += score + enemyScore;
  money += (score + enemyScore) / 3;

  drawPetLeveling("pong", petPongXP, enemyScore + score / 2, petPongLVL);

  petPongXP += enemyScore + score / 2;

  //petPongLVL++;  //for debug

  if (petPongXP > petPongLVL * 5) {
    petPongXP -= petPongLVL * 5;
    petPongLVL++;
  }

  if (score > enemyScore) {
    petMessage("aw dang it ggs");
  } else {
    petMessage("LMAO YOU SUCK AT THIS GAME");
  }

  score = 0;
  enemyScore = 0;
}