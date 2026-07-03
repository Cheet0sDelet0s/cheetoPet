#include "hardwareConfig.h"
#include "os.h"
#include "games.h"

void pong() {
  int ballX = 64;
  int ballY = 64;
  float ballVX = 2.0;
  float ballVY = 1.0;

  int playerScore = 0;
  int enemyScore = 0;

  int paddleX = 0;
  int paddleY = 16;
  int enemyX = 126;
  int enemyY = 16;

  int paddleHeight = 20;

  bool exit = false;

  int difficulty = promptDifficulty();

  drawCheckerboard(8);

  while (!exit) {
    ballX += ballVX;
    ballY += ballVY;
    ballX = round(ballX);
    ballY = round(ballY);
    
    updateGyro();
    angleY = constrain(angleY, -32, 32);
    updateButtonStates();

    if (leftButtonState) {
      angleY = 0;
    }
    
    paddleY = angleY * 2 + 64;

    // check player paddle collision
    if (ballX <= paddleX + 3 && ballY > paddleY - 2 && ballY < paddleY + paddleHeight + 2) {
      ballVX *= -3;
      float diff = paddleHeight - (ballY - paddleY) / 2;
      ballVY = round(diff * 10) / 10;
      ballX += 4;
    }

    if (ballX >= SCREEN_WIDTH) {
      //player scored
      playerScore++;
      ballVX *= -0.5;
      ballX -= 15;
    }

    if (ballX < 0) {
      //enemy scored
      enemyScore++;
      ballVX *= -0.5;
      ballX += 15;
    }

    if (ballY >= SCREEN_HEIGHT || ballY <= 1) {
      ballVY *= -1;
    }

    ballVX = constrain(ballVX, -8, 8);
    ballVY = constrain(ballVY, -8, 8);

    ballVX *= 1.01;
    ballVY *= 1.01;

    if (ballY <= 0) {
      ballY = 2;
    }
    if (ballY >= SCREEN_HEIGHT + 1) {
      ballY = SCREEN_HEIGHT - 2;
    }

    if (ballVX > 0) {
      int distance = ballY - (enemyY + paddleHeight / 2);
      enemyY += distance / (1.65 * (4 - difficulty / 4) / 2);
    }

    // check enemy paddle collision
    if (ballX > enemyX - 3 && ballY > enemyY - 2 && ballY < enemyY + paddleHeight + 2) {
      ballVX *= -3;
      float diff = paddleHeight - (ballY - enemyY) / 2;
      ballVY = round(diff * 10) / 10;
      ballX -= 4;
    }

    if (playerScore > 9 || enemyScore > 9 || rightButtonState) {
      exit = true;
    }

    display.clearDisplay();
    display.fillCircle(ballX, ballY, 2, SH110X_WHITE);
    display.fillRect(paddleX, paddleY, 2, paddleHeight, SH110X_WHITE);
    display.fillRect(enemyX, enemyY, 2, paddleHeight, SH110X_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(playerScore);
    display.setCursor(120, 0);
    display.print(enemyScore);
    display.display();

    updatePreviousStates();
    batteryManagement();
    updatePet();
    delay(7 * (4 - difficulty)); //quicker delay for higher difficulty
  }
  money += playerScore / 2 + enemyScore / 2;
  petFun += round(playerScore / 3 + enemyScore / 3);
}

void safeCrack() {
  const int steps = 3;
  int targets[steps];
  int directions[steps];   // -1 = CCW, +1 = CW

  targets[0] = random(-180, 181);
  targets[1] = random(-180, 181);
  targets[2] = random(-180, 181);

  directions[0] = -1;
  directions[1] =  1;
  directions[2] = -1;

  int step = 0;
  float lastAngle = angleZ;
  bool armed = false;
  int clickTimer = 0;

  drawCheckerboard(8);

  display.setTextSize(2);
  display.setFont(NULL);
  angleZ = 0;
  while (step < steps) {
    updateGyro();
    float angle = -angleZ;

    // wrap angle
    if (angle <= -180) angle += 360;
    if (angle > 180)   angle -= 360;

    // rotation delta
    float delta = angle - lastAngle;
    if (delta > 180)  delta -= 360;
    if (delta < -180) delta += 360;

    int rotationDir = (delta > 0) ? 1 : (delta < 0) ? -1 : 0;
    lastAngle = angle;

    // angular difference
    int diff = abs(angle - targets[step]);
    if (diff > 180) diff = 360 - diff;

    // ----- STATE LOGIC -----

    // disarm if wrong direction
    if (armed && rotationDir != directions[step]) {
      armed = false;
    }

    // arm only when rotating correctly
    if (!armed && rotationDir == directions[step]) {
      armed = true;
    }

    // trigger once per step
    if (armed && diff < 2) {
      step++;
      armed = false;
      clickTimer = 12;  // visual feedback duration
    }

    // ----- DRAWING -----

    float rad = angle * DEG_TO_RAD;
    int lineX = 64 + 20 * cos(rad);
    int lineY = 64 + 20 * sin(rad);

    display.clearDisplay();

    if (clickTimer > 0) {
      clickTimer--;
      display.fillCircle(64, 64, 40, SH110X_WHITE);
      display.drawLine(64, 64, lineX, lineY, SH110X_BLACK);
    } else {
      display.drawCircle(64, 64, 40, SH110X_WHITE);
      display.drawLine(64, 64, lineX, lineY, SH110X_WHITE);
    }
    
    if (step == 0 || step == 2) {
      drawCenteredText("<", 0);
      drawCenteredText(">", 114);
    } else {
      drawCenteredText(">", 0);
      drawCenteredText("<", 114);
    }

    display.display();
    updatePet();
    batteryManagement();
    delay(5);
  }
  money += 5;
  petFun += 5;
  display.setTextSize(1);
}