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

int itemBeingPlaced = -1;
bool startHandlingPlacing = false;

DateTime now;
DRAM_ATTR int lastSecond = -1;

//game library
int gameLibrary[8] = { 0 };
int gameLibraryCount = 1;
//                             0        1           2             3           4
const String gameNames[5] = {"pong", "shooty", "flappy bur", "bubblebox", "3d test"};

void placeItemFromInventory(int id) {
  int index = indexOfList(inventory, inventoryItems, id);

  if (index != -1) {
    removeFromList(inventory, inventoryItems, index);
    inventoryItems--;
  }

  itemBeingPlaced = id;
  startHandlingPlacing = true;

  setCurrentMenu("pet");
}

void purchaseItem(int id, float price) {
  if (money >= price) {
    addToList(inventory, inventoryItems, 8, id);
    money -= price;
    showPopup("added to inventory!", 1000);
  }
}

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

    if (itemToPackUp == i) {
      display.drawRoundRect(x - 2, y - 2, bmp.width + 4, bmp.height + 4, 2, SH110X_WHITE);
    }
  }

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
      petSitTimer--;
    } else {
      drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].stillID + 1, SH110X_BLACK);
      drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].stillID, SH110X_WHITE);
    }
  } else {
    if (petMoveAnim < 3) {
      drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].walk1ID + 1, SH110X_BLACK);
      drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].walk1ID, SH110X_WHITE);
    } else {
      drawBitmapFromList(drawX - 1, drawY - 1, petDir, pets[petNumber].walk2ID + 1, SH110X_BLACK);
      drawBitmapFromList(drawX, drawY, petDir, pets[petNumber].walk2ID, SH110X_WHITE);
    }
  }
}

void handlePetButtons() {
  if (buttonPressedThisFrame(1)) {
    if (itemToPackUp == -1) { // if not in edit mode just go to main menu when B pressed
      setCurrentMenu("main menu");
    } else {
      itemToPackUp--;
      if (itemToPackUp < 0) {
        itemToPackUp = currentAreaPtr->size() - 1;
      }
    }    
  } else if (buttonPressedThisFrame(2)) { 
    if (itemToPackUp == -1) {

    } else {
      itemToPackUp++;
      if (itemToPackUp > currentAreaPtr->size() - 1) {
        itemToPackUp = 0;
      }
    }
  } else if (buttonPressedThisFrame(3)) {
    if (itemBeingPlaced == -1 && itemToPackUp == -1) {
      setCurrentMenu("pet menu");
    } else if (itemToPackUp != -1) {
      updateAreaPointers();
      ItemList item = (*currentAreaPtr)[itemToPackUp];
      int id = item.type;
      addToList(inventory, inventoryItems, 16, id);
      currentAreaPtr->erase(currentAreaPtr->begin() + itemToPackUp); // holy shit this is really bad
      itemToPackUp = -1;
    }    
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

void handleItemPlacing() {
  updateGyro();
  
  angleX = constrain(angleX, -32, 32);
  angleY = constrain(angleY, -32, 32);

  int cursorX = angleX * 2 + 64;
  int cursorY = angleY * 2 + 64;

  drawBitmapFromList(cursorX, cursorY, 1, itemBeingPlaced, SH110X_WHITE);

  if (buttonPressedThisFrame(3)) {
    updateAreaPointers();

    ItemList newItem = {
      itemBeingPlaced,
      cursorX,
      (itemBeingPlaced == 5 ? 27 : cursorY)
    };

    currentAreaPtr->push_back({newItem});

    itemBeingPlaced = -1;
  }
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

  if (itemBeingPlaced != -1) {
    handleItemPlacing();
  } else if (messageDisplayTime < messageMaxTime) {
      drawPetMessage(); // draw speech bubble on pet if it hasn't expired
  }                   // AND not currently placing an item.
  
  drawPet(userPet, petX, petY); // draw the pet to the display in its current state
  
  drawEmotionUI(); // draw pet stats in top left (hunger sleep etc)

  drawParticles(); // render particles on screen, updating particles is seperate

  if (itemToPackUp != -1) {
    display.setFont(NULL);
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.setTextSize(1);
    drawCenteredText("choose item to pick up", 0);
  }
}