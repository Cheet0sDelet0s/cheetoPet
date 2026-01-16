#include "pet.h"

int saveFileVersion = 3;

int petTypes = sizeof(pets) / sizeof(pets[0]);

// AREA VARIABLES
std::vector<ItemList> homePlot = {
  {7, 98, 4}
};

std::vector<ItemList> outsidePlot = {
    {34, 40, 50},
    {35, 60, 70},
    {33, 96, 5},
    {36, 46, 6},
    {42, 29, 25}
};

//item inventory
DRAM_ATTR int inventory[8] = {};
DRAM_ATTR int inventoryItems = 0;

DRAM_ATTR int itemBeingPlaced = -1;
DRAM_ATTR bool startHandlingPlacing = false;

//food inventory
DRAM_ATTR int foodInventory[8] = { 18 };
DRAM_ATTR int foodInventoryItems = 1;

DRAM_ATTR bool handleFoodPlacing = false;

DateTime now;
DRAM_ATTR int lastSecond = -1;

//game library
DRAM_ATTR int gameLibrary[8] = { 0 };
DRAM_ATTR int gameLibraryCount = 1;
//                             0        1           2             3           4
const String gameNames[5] = {"pong", "shooty", "flappy bur", "bubblebox", "3d test"};

void petMessage(String message) {
  message.replace("{PETNAME}", petName);
  currentPetMessage = message;
  messageDisplayTime = 0;
  int msgLength = currentPetMessage.length();
  messageMaxTime = constrain((msgLength * 15) / 2, 1, 3000);

  for (int i = 0; i < msgLength; i++) {
    int letterNumber = currentPetMessage[i] - 'a';
    int frequency = 400 + letterNumber * 4;
    queueTone(frequency, 50);
  }

  // Spawn particles around the pet's mouth
  float baseAngle = petDir == 1 ? 0 : PI; // Base angle depends on pet direction
  float angleStep = 15 * DEG_TO_RAD; // 15 degrees in radians
  float speed = 1.5; // Particle speed

  for (int i = -1; i <= 1; i++) { // Generate 3 particles, spaced 15 degrees apart
    float angle = baseAngle + i * angleStep; // Adjust angle for each particle
    float vx = cos(angle) * speed; // Velocity in x direction
    float vy = sin(angle) * speed; // Velocity in y direction
    createParticle(3, petX + 8 * petDir, petY + 4, vx, vy, random(16, 24)); // Spawn particle near pet's mouth
  }
}

bool removeFromList(int list[], int& itemCount, int index) {
  if (index < 0 || index >= itemCount) {
    return false;  // Invalid index
  }

  // Shift elements left to fill the gap
  for (int i = index; i < itemCount - 1; i++) {
    list[i] = list[i + 1];
  }

  return true;  // Success
}

void updateAreaPointers() {
  if (currentArea == 0) {
    currentAreaPtr = &homePlot;
  } else if (currentArea == 1) {
    currentAreaPtr = &outsidePlot;
  }
}

bool checkItemIsPlaced(int item) {
  updateAreaPointers();
  for (int i = 0; i < currentAreaPtr->size(); i++) {
    if ((*currentAreaPtr)[i].type == item) {
      return true;
    }
  }
  return false;
}

int findIndexByType(const std::vector<ItemList>& vec, uint8_t type) {
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].type == type) {
            return i;  // found
        }
    }
    return -1; // not found
}

int countTypes(const std::vector<ItemList>& vec, uint8_t type) {
  int count = 0;
  for (size_t i = 0; i < vec.size(); i++) {
      if (vec[i].type == type) {
          count++;
      }
  }
  return count;
}

void sitPet(int time, int type) {
  petSitTimer = time;
  petSitType = type;
}


void startMovingPet(int x, int y, int speed) {
  if (petSitTimer < 1) {
    petMoveX = x;
    petMoveY = y;
    petMoveSpeed = speed;
    petMoveAnim = 0;
    movePet = true;
  }
}

void updatePetNeeds() {
  now = rtc.now();
  if (now.second() != lastSecond) {
    lastSecond = now.second();

    if (now.second() % 60 == 0) {  // Once per minute
      petHunger -= 1;
      petFun -= 1;
      petSleep -= 5;
      petPoop += 10;
    }
  }
}

void updateMood() {
  float hungerMultipler = constrain(petHunger / 50.00, 0.00, 2.00);
  float funMultipler = constrain(petFun / 50.00, 0.00, 2.00);
  float sleepMultipler = constrain(petSleep / 50.00, 0.00, 2.00);

  float cleanlinessMultipler = 2.00;

  updateAreaPointers();

  cleanlinessMultipler = constrain(cleanlinessMultipler - countTypes(*currentAreaPtr, 38) / 2.00, 0.00, 2.00);  

  petMood = ((hungerMultipler + funMultipler + sleepMultipler + cleanlinessMultipler) / 4.00) * 50.00;
}

void updatePetMovement() {
  int xDiff = petMoveX - petX;
  xDiff = xDiff / abs(xDiff);

  petDir = xDiff;

  int yDiff = petMoveY - petY;
  yDiff = yDiff / abs(yDiff);
  
  petX += xDiff * petMoveSpeed;
  petY += yDiff * petMoveSpeed;

  if (xDiff == 1) {
    if (petX > petMoveX) {
      petX = petMoveX;
    }
  } else {
    if (petX < petMoveX) {
      petX = petMoveX;
    }
  }

  if (yDiff == 1) {
    if (petY > petMoveY) {
      petY = petMoveY;
    }
  } else {
    if (petY < petMoveY) {
      petY = petMoveY;
    }
  }

  // if (random(1, 10) == 1) {
  //   createParticle(1, petX, petY, xDiff * -1, yDiff * -1, 20);
  // }
  
  if (petX == petMoveX && petY == petMoveY) {
    movePet = false;
  }
  petMoveAnim++;
  if (petMoveAnim > 4) {
    petMoveAnim = 0;
  }
}

void killPet(String deathReason = "") {
    spiralFill(SH110X_WHITE);
    delay(500);
    display.clearDisplay();
    display.display();
    delay(1000);
    const BitmapInfo& bmp = bitmaps[25];
    display.drawBitmap(49, 94, bmp.data, bmp.width, bmp.height, SH110X_WHITE);
    display.display();
    delay(1000);
    display.setCursor(0,0);
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.println("your pet\ndied");
    display.display();
    delay(500);
    display.setTextSize(1);
    display.println("after it ");
    display.display();
    delay(500);
    display.println(deathReason);
    display.display();
    delay(500);
    display.println("we are sorry\nfor your loss.");
    display.display();
    delay(500);
    display.println("press A to\nrestart.");
    display.display();
    updateButtonStates();
    while(!rightButtonState) {updateButtonStates();}
    esp_restart();
}

void drawAreaItems() {
  display.drawFastHLine(0, 42, 127, SH110X_WHITE);

  updateAreaPointers();
  updateButtonStates();


  for (int i = 0; i < currentAreaPtr->size(); i++) {
    ItemList currentItem = (*currentAreaPtr)[i];

    uint8_t type = currentItem.type;
    if (type == 0) continue;  // stop foolish behavoir1!1!!!1!

    const BitmapInfo& bmp = bitmaps[type];
    int x = currentItem.x;
    int y = currentItem.y;

    display.drawBitmap(x, y, bmp.data, bmp.width, bmp.height, SH110X_WHITE);

    if (type == 5) {
      display.fillRect(x + 6, y, 3, -27, SH110X_WHITE);
    }

    // if (detectCursorTouch(x, y, bmp.width, bmp.height) && currentItem.type != 36 && currentItem.type != 42) {
    //   if (rightButtonState && loadIndicator > 9) {
    //     addToList(inventory, inventoryItems, 8, currentItem.type);
    //     currentAreaPtr->erase(currentAreaPtr->begin()+i);
    //     loadIndicator = 0;
    //   } else if (rightButtonState) {
    //     loadIndicator++;
    //   }
    // }
    
    // if (detectCursorTouch(x, y, bmp.width, bmp.height) && currentItem.type == 42) {
    //   if (rightButtonState && loadIndicator > 9) {
    //     openNews();
    //     loadIndicator = 0;
    //   } else if (rightButtonState) {
    //     loadIndicator++;
    //   }
    // }
  }

  // updateDoorDimensions(exitLocations[currentArea]);

  // if (exitLocations[currentArea] != 0) {
  //   display.fillRect(doorX, doorY, doorW, doorH, SH110X_WHITE);
  // }

  // if (detectCursorTouch(doorX, doorY, doorW, doorH)) {
  //   if (rightButtonState && loadIndicator > 9) {
  //     currentArea = exitLinks[currentArea];
  //     loadIndicator = 0;
  //     arrowAnimation();
  //     updateDoorDimensions(exitLocations[currentArea]);
  //     petX = doorX;
  //     petY = doorY;
  //   } else if (rightButtonState) {
  //     loadIndicator++;
  //   }
  // }

  for (int i = 0; i < amountFoodPlaced; i++) {
    const BitmapInfo& bmp = bitmaps[placedFood[i]];
    display.drawBitmap(placedFoodX[i], placedFoodY[i], bmp.data, bmp.width, bmp.height, SH110X_WHITE);
  }

}

void drawPetMessage() { 
  const int charWidth = 6;  // width of a character in pixels
  const int lineHeight = 11; // height of a line in pixels
  const int maxCharsPerLine = 8;  // max characters per line before wrapping

  // Split currentPetMessage into wrapped lines
  std::vector<String> lines;
  String remaining = currentPetMessage;
  while (remaining.length() > 0) {
    int wrapAt = maxCharsPerLine;
    if (remaining.length() <= maxCharsPerLine) {
      lines.push_back(remaining);
      break;
    }

    // Try to break at the last space before wrap limit
    int lastSpace = remaining.lastIndexOf(' ', maxCharsPerLine);
    if (lastSpace == -1) lastSpace = maxCharsPerLine;  // no space, hard break

    lines.push_back(remaining.substring(0, lastSpace));
    remaining = remaining.substring(lastSpace);
    remaining.trim(); // remove leading space
  }

  int lineCount = lines.size();
  int bubbleHeight = lineCount * lineHeight + 4;
  int bubbleWidth = 0;
  for (String& line : lines) {
    bubbleWidth = max(bubbleWidth, (int)line.length());
  }
  bubbleWidth = bubbleWidth * charWidth + 4;

  // Position bubble relative to pet
  int bubbleX;
  int bubbleDirection;
  if (petX < 64) {
    bubbleX = petX + 17;
    bubbleDirection = 1;
  } else {
    bubbleX = petX;
    bubbleDirection = -1;
  }
  int bubbleY = petY - 3;

  // Adjust if bubble is off-screen at the top
  int bubbleTop = bubbleY - bubbleHeight;
  if (bubbleTop < 0) {
    bubbleY += -bubbleTop; // Move it down by the amount it's off-screen
  }

  // Draw text
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);

  display.fillRect(
    bubbleDirection == 1 ? bubbleX : bubbleX - bubbleWidth,
    bubbleY - bubbleHeight,
    bubbleWidth,
    bubbleHeight - 3, // leave space for the tail
    SH110X_BLACK
  );

  for (int i = 0; i < lineCount; i++) {
    int lineX = (bubbleDirection == 1)
                  ? bubbleX + 2
                  : bubbleX - (2 + lines[i].length() * charWidth);
    int lineY = bubbleY - bubbleHeight + 2 + i * lineHeight;
    display.setCursor(lineX, lineY);
    display.print(lines[i]);
  }

  display.setTextColor(SH110X_WHITE); // restore default

  // Draw bubble
  int edgeX = bubbleX;
  int edgeW = bubbleWidth * bubbleDirection;

  // Left vertical line
  display.drawFastVLine(edgeX, bubbleY, -bubbleHeight + 1, SH110X_WHITE);

  // Right vertical line
  display.drawFastVLine(edgeX + edgeW, bubbleY - bubbleHeight + 1, bubbleHeight - 4, SH110X_WHITE);

  // Top horizontal line
  display.drawFastHLine(edgeX, bubbleY - bubbleHeight, edgeW - bubbleDirection, SH110X_WHITE);

  // Bottom portion of bubble (tail)
  display.drawFastHLine(edgeX + 5 * bubbleDirection, bubbleY - 5, (bubbleWidth - 6) * bubbleDirection, SH110X_WHITE);
  display.drawLine(edgeX, bubbleY, edgeX + 5 * bubbleDirection, bubbleY - 5, SH110X_WHITE);

  messageDisplayTime++;

  updateButtonStates();

  // if (detectCursorTouch(bubbleX - (bubbleDirection == 1 ? 0 : bubbleWidth), bubbleY - bubbleHeight, bubbleWidth, bubbleHeight) && rightButtonState) {
  //   messageDisplayTime = messageMaxTime + 1; // close bubble

  //   float speed = 2; // particle speed, adjust as needed
  //   int topLeftX = bubbleDirection == 1 ? bubbleX : bubbleX - bubbleWidth;
  //   int topLeftY = bubbleY - bubbleHeight;
  //   int topRightX = bubbleDirection == 1 ? bubbleX + bubbleWidth : bubbleX;
  //   int topRightY = bubbleY - bubbleHeight;
  //   int bottomLeftX = bubbleDirection == 1 ? bubbleX : bubbleX - bubbleWidth;
  //   int bottomLeftY = bubbleY - 3; // bottom edge
  //   int bottomRightX = bubbleDirection == 1 ? bubbleX + bubbleWidth : bubbleX;
  //   int bottomRightY = bubbleY - 3;

  //   createParticle(3, topLeftX, topLeftY, -speed, -speed, 5);
  //   createParticle(3, topRightX, topRightY, speed, -speed, 5);
  //   createParticle(3, bottomLeftX, bottomLeftY, -speed, speed, 5);
  //   createParticle(3, bottomRightX, bottomRightY, speed, speed, 5);
  // }
}

void drawPet(int petNumber, int drawX, int drawY) {
  if (!movePet) {
    if (petSitTimer > 0) {
      switch (petSitType) {
        case 0: // normal sit
          drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].sitID + 1, SH110X_BLACK);
          drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].sitID, SH110X_WHITE);
          break;
        case 1: // marshmellow
          drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].marshmellowID + 1, SH110X_BLACK);
          drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].marshmellowID, SH110X_WHITE);
          break;
        case 2: // standing, just staying still
          drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].stillID + 1, SH110X_BLACK);
          drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].stillID, SH110X_WHITE);
          break;
      }
      // drawBitmapFromList(drawX - 1, drawY - 1, petDir, petSitType + 1, SH110X_BLACK);
      // drawBitmapFromList(drawX, drawY, petDir, petSitType, SH110X_WHITE);
      petSitTimer--;
    } else {
      drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].stillID + 1, SH110X_BLACK);
      drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].stillID, SH110X_WHITE);
      // drawBitmapWithDirection(drawX - 1, drawY - 1, petDir, pet_gooseStillBigMask, 18, 28, SH110X_BLACK);
      // drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseStillBig, 16, 26, SH110X_WHITE);
    }
  } else {
    if (petMoveAnim < 3) {
      drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].walk1ID + 1, SH110X_BLACK);
      drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].walk1ID, SH110X_WHITE);
      // drawBitmapWithDirection(drawX - 1, drawY - 1, petDir, pet_gooseWalkMask, 18, 27, SH110X_BLACK);
      // drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseWalk, 16, 26, SH110X_WHITE);
    } else {
      drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].walk2ID + 1, SH110X_BLACK);
      drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].walk2ID, SH110X_WHITE);
      // drawBitmapWithDirection(drawX - 1, drawY, petDir, pet_gooseWalk2Mask, 19, 26, SH110X_BLACK);
      // drawBitmapWithDirection(drawX, drawY+1, petDir, pet_gooseWalk2, 17, 25, SH110X_WHITE);
    }
  }
}

void handlePetButtons() {
  if (buttonPressedThisFrame(1)) {
    setCurrentMenu("main menu");
  } else if (buttonPressedThisFrame(3)) {
    setCurrentMenu("pet menu");
  }
}

void drawEmotionUI() {
  display.setCursor(0, -2);
  display.setTextSize(1);
  
  // if (detectCursorTouch(0, 0, 30, 34)) {
  //   display.setTextColor(SH110X_BLACK);
  //   display.fillRect(0, 0, 28, 34, SH110X_WHITE);
  //   updateButtonStates();
  //   if (rightButtonState) {
  //     openEmotionMenu();
  //   }
  // } else {
  //   display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  // }
  
  display.setFont(&Picopixel);
  display.print("MOOD: ");
  display.print(int(round(petMood)));
  if (petMood < 40) {
    display.println("!");
  } else {
    display.println("");
  }
  display.print("HUN: ");
  display.print(petHunger);
  if (petHunger < 30) {
    display.println("!");
  } else {
    display.println("");
  }
  display.print("FUN: ");
  display.print(petFun);
  if (petFun < 40) {
    display.println("!");
  } else {
    display.println("");
  }
  display.print("SLP: ");
  display.print(petSleep);
  if (petSleep < 30) {
    display.println("!");
  } else {
    display.println("");
  }
  display.print("$");
  display.print(money);

  display.setTextColor(SH110X_WHITE);
  display.setFont(NULL);
}

void updatePet() {

  if (totalG > 11) {   //kinda funny but annoying. kills the pet if the device experiences over 11 g's of force. hard to do on accident unless you're in a fighter jet or something
    killPet("got shaken to death"); 
  }

  unsigned long currentMillis = millis(); // this is for non-blocking delta time, you can't use delay() most of the time as it blocks the whole program

  if (currentMillis - previousMillis >= interval) { // if the interval has happened, move the pet
    previousMillis = currentMillis;     //interval passed
    
    if (movePet) {
      updatePetMovement(); //moves pet towards its desired position
    }
  }

  tree->tick();  //behaviour tree update

  DateTime now = rtc.now(); // get the current time from the RTC chip

  updatePetNeeds();
  updateMood();

  updateButtonStates(); // update button states

  updateParticles(); // update any particles spawned

  audioEngine(); // write current tone to speaker pin
}

void drawPetHome() {
  drawAreaItems();
  
  drawPet(userPet, petX, petY); // draw the pet to the display in its current state
  
  if (messageDisplayTime < messageMaxTime) {
    drawPetMessage(); // draw speech bubble on pet if it hasn't expired
  }
  
  drawEmotionUI();

  drawParticles();
}