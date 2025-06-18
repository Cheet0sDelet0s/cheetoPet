/*

cheetoPet - by olly jeffery / @Cheet0sDelet0s

an esp32 c3 based, feature-rich tamagotchi!

fully open source - do whatever you want with it!
you dont have to, but it would be great if you could credit me if you use any of this stuff!

*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <RTClib.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_NeoPixel.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "vector"
#include "bitmaps.h"
#include "petLines.h"
#include "eepromHandler.h"
#include "pong.h"
#include "veridium.h"

#define SDA_ALT 20
#define SCL_ALT 9
#define LED_PIN 8
#define LED_COUNT 1
#define SWITCH_PIN GPIO_NUM_0
#define BATT_PIN GPIO_NUM_3
const float voltageDividerRatio = 2.0;  // R1 + R2 = 2x actual
#define ADC_MAX 4095       // 12-bit ADC
#define REF_VOLTAGE 3.7    // ADC reference voltage

Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define SCREEN_WIDTH 128   // display width
#define SCREEN_HEIGHT 128  // display height
#define OLED_RESET -1      // can set an oled reset pin if desired

#define EEPROM_ADDRESS 0x57 // Check your chip
#define EEPROM_SIZE 4096    // AT24C32 = 4KB

/*FLASH AND DRAM
anything that is read/written to infrequently should go in flash. (like storage of computer)
anything that is used quite a bit goes in dram (like ram of computer)
this makes things faster and stops the flash of the chip wearing out after lots of use.
try to upload code as little as possible to prolong lifetime of the chip,
it can spontaneously die if you arent careful.
*/

const int leftButton = 5;
const int middleButton = 6;
const int rightButton = 7;

DRAM_ATTR unsigned long previousMillis = 0;
const long interval = 50;  

DRAM_ATTR DateTime now;

DRAM_ATTR int saveFileVersion = 1;

//buttons
DRAM_ATTR bool leftButtonState = false;
DRAM_ATTR bool middleButtonState = false;
DRAM_ATTR bool rightButtonState = false;
DRAM_ATTR bool powerSwitchState = false;

//menu system
DRAM_ATTR int firstOption = 0;
DRAM_ATTR int secondOption = 0;
DRAM_ATTR int thirdOption = 0;
DRAM_ATTR int depth = 0;
DRAM_ATTR int settingsOption = 0;
DRAM_ATTR int selectedField = 0; // 0=year, 1=month, 2=day, 3=hour, 4=minute
DRAM_ATTR bool editingTime = false;

//debug
DRAM_ATTR int handleItemsGotThrough = 0;

RTC_DS3231 rtc;
DateTime tempDateTime;
MPU9250_asukiaaa mpu(0x69);

DRAM_ATTR char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
DRAM_ATTR int lastSecond = -1;
DRAM_ATTR DateTime lastRunTime;
DRAM_ATTR int saveInterval = 5;

DRAM_ATTR int liveDataTimer = 0;

//item inventory
DRAM_ATTR int inventory[8] = {};
DRAM_ATTR int inventoryItems = 0;
DRAM_ATTR int placedHomeItems[30] = { 7 };
DRAM_ATTR int amountItemsPlaced = 1;
DRAM_ATTR int placedHomeItemsX[30] = { 98 };
DRAM_ATTR int placedHomeItemsY[30] = { 4 };
DRAM_ATTR int itemBeingPlaced = -1;
DRAM_ATTR bool startHandlingPlacing = false;

//food inventory
DRAM_ATTR int foodInventory[8] = { 18 };
DRAM_ATTR int foodInventoryItems = 1;
DRAM_ATTR int placedFood[10] = {};
DRAM_ATTR int amountFoodPlaced = 0;
DRAM_ATTR int placedFoodX[10] = {};
DRAM_ATTR int placedFoodY[10] = {};
DRAM_ATTR bool handleFoodPlacing = false;

//game library
DRAM_ATTR int gameLibrary[8] = { 0, 1 };
DRAM_ATTR int gameLibraryCount = 2;

const String gameNames[2] = {"pong", "helldivers II"};

Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

DRAM_ATTR float gyroXOffset = -0.40;
DRAM_ATTR float gyroYOffset = 3;
DRAM_ATTR float gyroZOffset = 1.5;
DRAM_ATTR float accelXOffset = 0;
DRAM_ATTR float accelYOffset = 0;
DRAM_ATTR float accelZOffset = 0;

DRAM_ATTR float angleX = 0;
DRAM_ATTR float angleY = 0;
DRAM_ATTR float angleZ = 0;
DRAM_ATTR float posX = 0;
DRAM_ATTR float posY = 0;
DRAM_ATTR float posZ = 0;
DRAM_ATTR float totalG = 0;
//cursor stuff
DRAM_ATTR int cursorX = 500;
DRAM_ATTR int cursorY = 500;
DRAM_ATTR bool shouldDrawCursor = false;
DRAM_ATTR float cursorTimer = 0;
DRAM_ATTR int cursorBitmap = 14;

DRAM_ATTR int uiTimer = 100;

DRAM_ATTR int petHunger = 50;
DRAM_ATTR int petFun = 50;
DRAM_ATTR int petSleep = 50;
DRAM_ATTR int petX = 64;
DRAM_ATTR int petY = 32;
DRAM_ATTR bool showPetMenu = false;
DRAM_ATTR bool movingPet = false;
DRAM_ATTR float money = 40;

DRAM_ATTR int petMoveX = 64;
DRAM_ATTR int petMoveY = 32;
DRAM_ATTR int petMoveSpeed = 0;
DRAM_ATTR bool movePet = false;
DRAM_ATTR int petMoveAnim = 0;
DRAM_ATTR int petDir = 1;
DRAM_ATTR int petSitTimer = 0;

DRAM_ATTR unsigned long lastUpdate = 0;

DRAM_ATTR String currentPetMessage;
DRAM_ATTR int messageDisplayTime = 0;
DRAM_ATTR int messageMaxTime = 0;

DRAM_ATTR int constructionShopItems[] = { 3, 4, 5, 6, 7, 19 };
DRAM_ATTR float constructionShopPrices[] = { 5, 2.50, 7.50, 10, 12.50, 4 };
DRAM_ATTR int constructionShopLength = sizeof(constructionShopItems) / sizeof(constructionShopItems[0]);

DRAM_ATTR int foodShopItems[] = { 16, 18 };
DRAM_ATTR float foodShopPrices[] = { 1.50, 1.50};
DRAM_ATTR int foodShopLength = sizeof(foodShopItems) / sizeof(foodShopItems[0]);

//settings
DRAM_ATTR int gyroSensitivityX = 2;
DRAM_ATTR int gyroSensitivityY = 3;
DRAM_ATTR int gyroSensitivityZ = 3;
DRAM_ATTR int loopDelay = 5;

DRAM_ATTR SaveGame currentSaveGame;

void petMessage(String message) {
  currentPetMessage = message;
  messageDisplayTime = 0;
  messageMaxTime = constrain((currentPetMessage.length() * 15) / 2, 1, 3000);
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

void drawCheckerboard(uint8_t squareSize = 8) {
  bool color;
  for (uint8_t y = 0; y < 128; y += squareSize) {
    color = (y / squareSize) % 2;  // Alternate row start
    for (uint8_t x = 0; x < 128; x += squareSize) {
      display.fillRect(x, y, squareSize, squareSize, color ? SH110X_WHITE : SH110X_BLACK);
      color = !color;  // Alternate color for each square
      }
    display.display();
  }
  display.display();
  delay(300);
}

bool isInArray(int item, int arr[], int arrSize) {
  for (int i = 0; i < arrSize; i++) {
    if (arr[i] == item) {
      return true;
    }
  }
  return false;
}

void drawBitmapFromList(int16_t x, int16_t y, int dir, const uint8_t bitmapID, uint16_t color) {
  const BitmapInfo& bmp = bitmaps[bitmapID];

  drawBitmapWithDirection(x, y, dir, bmp.data, bmp.width, bmp.height, color);
}

void drawBitmapWithDirection(int16_t x, int16_t y, int dir, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
  if (dir == 1) {
    display.drawBitmap(x, y, bitmap, w, h, color);
  } else {
    drawBitmapFlippedX(x, y, bitmap, w, h, color);
  }
}

void drawCenteredText(Adafruit_GFX &display, const String &text, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  
  // Get the size of the text
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  
  // Calculate the centered X position
  int16_t x = (display.width() - w) / 2;

  // Draw the text at the centered X position
  display.setCursor(x, y);
  display.print(text);
}

void spiralFill(Adafruit_GFX &d, uint16_t color) {
  int x0 = 0;
  int y0 = 0;
  int x1 = SCREEN_WIDTH - 1;
  int y1 = SCREEN_HEIGHT - 1;

  while (x0 <= x1 && y0 <= y1) {
    // Top edge
    for (int x = x0; x <= x1; x++) {
      d.drawPixel(x, y0, color);
    }
    y0++;

    // Right edge
    for (int y = y0; y <= y1; y++) {
      d.drawPixel(x1, y, color);
    }
    x1--;

    // Bottom edge
    if (y0 <= y1) {
      for (int x = x1; x >= x0; x--) {
        d.drawPixel(x, y1, color);
      }
      y1--;
    }

    // Left edge
    if (x0 <= x1) {
      for (int y = y1; y >= y0; y--) {
        d.drawPixel(x0, y, color);
      }
      x0++;
    }
    display.display();
    delay(5);  // Adjust delay for animation speed
  }
}

void sitPet(int time) {
  petSitTimer = time;
}

void saveGameToEEPROM(bool showUI = true) {
  if (showUI) {
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    drawCenteredText(display, "saving...", 60);
    display.display();
  }
  
  saveGameToRam();

  writeStructEEPROM(EEPROM_ADDRESS, currentSaveGame);
  
  if (showUI) {
    drawCenteredText(display, "saved!", 68);
    display.display();
  }
  delay(500);
}

void saveGameToRam() {
  currentSaveGame.hunger = petHunger;
  currentSaveGame.sleep = petSleep;
  currentSaveGame.fun = petFun;
  currentSaveGame.money = money;
  currentSaveGame.pongXP = petPongXP;
  currentSaveGame.pongLVL = petPongLVL;

  for (int i = 0; i < 8; i++) {
    currentSaveGame.invent[i] = inventory[i];
  }

  currentSaveGame.inventItems = inventoryItems;

  for (int i = 0; i < 30; i++) {
    currentSaveGame.placed[i] = placedHomeItems[i];
  }

  currentSaveGame.placedItems = amountItemsPlaced;

  for (int i = 0; i < 30; i++) {
    currentSaveGame.placedX[i] = placedHomeItemsX[i];
  }

  for (int i = 0; i < 30; i++) {
    currentSaveGame.placedY[i] = placedHomeItemsY[i];
  }

  for (int i = 0; i < 8; i++) {
    currentSaveGame.foodInv[i] = foodInventory[i];
  }

  currentSaveGame.foodInvItems = foodInventoryItems;
  currentSaveGame.saveVersion = saveFileVersion;
}

void loadGameFromRam() {
  petHunger = currentSaveGame.hunger;
  petSleep = currentSaveGame.sleep;
  petFun = currentSaveGame.fun;
  money = currentSaveGame.money;
  petPongXP = currentSaveGame.pongXP;
  petPongLVL = currentSaveGame.pongLVL;
  
  for (int i = 0; i < 8; i++) {
    inventory[i] = currentSaveGame.invent[i];
  }

  inventoryItems = currentSaveGame.inventItems;
  
  for (int i = 0; i < 30; i++) {
    placedHomeItems[i] = currentSaveGame.placed[i];
  }

  amountItemsPlaced = currentSaveGame.placedItems;
  
  for (int i = 0; i < 30; i++) {
    placedHomeItemsX[i] = currentSaveGame.placedX[i];
  }

  for (int i = 0; i < 30; i++) {
    placedHomeItemsY[i] = currentSaveGame.placedY[i];
  }

  for (int i = 0; i < 8; i++) {
    foodInventory[i] = currentSaveGame.foodInv[i];
  }

  foodInventoryItems = currentSaveGame.foodInvItems;
  int saveVersion = currentSaveGame.saveVersion;
}

void loadGameFromEEPROM() {
  display.setTextColor(SH110X_BLACK, SH110X_WHITE);
  drawCenteredText(display, "loading game...", 60);
  display.display();
  currentSaveGame = readStructEEPROM(EEPROM_ADDRESS);

  loadGameFromRam();

  drawCenteredText(display, "loaded!", 68);
  display.display();
  delay(500);
}

void printSaveGame(const SaveGame& save) {
  Serial.println("=== SaveGame ===");

  Serial.print("Hunger: "); Serial.println(save.hunger);
  Serial.print("Sleep: "); Serial.println(save.sleep);
  Serial.print("Fun: "); Serial.println(save.fun);
  Serial.print("Money: "); Serial.println(save.money);
  Serial.print("Pong XP: "); Serial.println(save.pongXP);
  Serial.print("Pong LVL: "); Serial.println(save.pongLVL);
  
  Serial.print("Inventory Items: "); Serial.println(save.inventItems);
  Serial.print("Inventory: ");
  for (uint8_t i = 0; i < save.inventItems && i < 8; i++) {
    Serial.print(save.invent[i]);
    Serial.print(i < save.inventItems - 1 ? ", " : "\n");
  }

  Serial.print("Food Inventory Items: "); Serial.println(save.foodInvItems);
  Serial.print("Food Inventory: ");
  for (uint8_t i = 0; i < save.foodInvItems && i < 8; i++) {
    Serial.print(save.foodInv[i]);
    Serial.print(i < save.foodInvItems - 1 ? ", " : "\n");
  }

  Serial.print("Placed Items: "); Serial.println(save.placedItems);
  for (uint8_t i = 0; i < save.placedItems && i < 30; i++) {
    Serial.print("  Placed["); Serial.print(i); Serial.print("]: ");
    Serial.print(save.placed[i]);
    Serial.print(" at (");
    Serial.print(save.placedX[i]);
    Serial.print(", ");
    Serial.print(save.placedY[i]);
    Serial.println(")");
  }

  Serial.print("Save Version: "); Serial.println(save.saveVersion);

  Serial.println("================");
}


void drawBitmapFlippedX(int16_t x, int16_t y,
                                const uint8_t *bitmap, int16_t w, int16_t h,
                                uint16_t color) {
  int byteWidth = (w + 7) / 8; // Bitmap width in bytes

  for (int16_t j = 0; j < h; j++) {
    for (int16_t i = 0; i < w; i++) {
      int16_t flipped_i = w - 1 - i;
      uint8_t byte = pgm_read_byte(bitmap + j * byteWidth + (flipped_i / 8));
      if (byte & (0x80 >> (flipped_i % 8))) {
        display.drawPixel(x + i, y + j, color);
      }
    }
  }
}

void mpu9250_sleep() {
  Wire.beginTransmission(0x68);  // Default MPU9250 I2C address
  Wire.write(0x6B);              // PWR_MGMT_1 register
  Wire.write(0x40);              // Set SLEEP bit (bit 6)
  Wire.endTransmission();
}

void mpu9250_wake() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x01);  // Use PLL with X-axis gyroscope as clock source
  Wire.endTransmission();
}

void printPlacedHomeItems() {
  for (int i = 0; i < amountItemsPlaced; i++) {
    Serial.print(placedHomeItems[i]);
    Serial.print(" ");
  }
  Serial.println();
}

bool addToList(int list[], int& itemCount, int maxSize, int value) {
  if (itemCount < maxSize) {
    list[itemCount++] = value;
    return true;
  } else {
    return false;
  }
}

int indexOf(int array[], int length, int target) {
  for (int i = 0; i < length; i++) {
    if (array[i] == target) {
      return i;
    }
  }
  return -1;  // Not found
}

void setup() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  if (digitalRead(SWITCH_PIN) == LOW) {
    esp_deep_sleep_start();
  }
  pinMode(BATT_PIN, INPUT);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(middleButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  Serial.begin(115200);
  analogReadResolution(12);

  Wire.begin(SDA_ALT, SCL_ALT);
  display.begin(0x3C, true);
  // delay(500);
  // display.display();
  // delay(2000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);
  display.println("cheetoPet");
  display.setTextSize(1);
  display.println("welcome to your\nsecond life!\n");
  display.println("loading modules\n\n");
  display.print("made by\nCheet0sDelet0s");
  display.display();
  delay(500);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    display.clearDisplay();
    display.println("bro i cant find the rtc module you are screwed");
    display.display();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    // Set the RTC to the current date & time
    rtc.adjust(DateTime(2025, 6, 6, 7, 53, 0));
    display.clearDisplay();
    display.println("rtc module lost power! time and date has been reset. oh dear. booting in 5 secs");
    display.display();
    delay(5000);
  }

  mpu.setWire(&Wire);
  mpu.beginGyro();
  mpu.beginAccel();

  rgb.begin();            // Initialize
  rgb.setBrightness(50);  // Optional: reduce brightness
  rgb.show();             // Initialize all pixels to 'off'

  lastUpdate = millis();

  while (1) {
      updateButtonStates();
      if (leftButtonState) {
      //load game
      loadGameFromEEPROM();
      break;
    } else if (rightButtonState) {
      saveGameToEEPROM();
      break;
    } else if (middleButtonState) {
      break;
    } else {
      display.clearDisplay();
      display.setTextSize(2);
      drawCenteredText(display, "cheetoPet", 0);
      display.setTextSize(1);
      drawCenteredText(display, "what would you like", 20);
      drawCenteredText(display, "to do?", 29);
      display.setCursor(0, 118);
      display.print("B: continue");
      display.setCursor(91, 118);
      display.print("new: A");
      display.display();
    }
  }
  
}

void waitForSelectRelease() {
  const unsigned long debounceDelay = 50;  // milliseconds
  unsigned long lastStableTime = millis();

  // Wait until the button is released and stable
  while (true) {
    if (digitalRead(rightButton) == HIGH) {
      // Button is released, check how long it's been stable
      if (millis() - lastStableTime >= debounceDelay) {
        break;  // Released and stable
      }
    } else {
      // Button pressed again — reset the timer
      lastStableTime = millis();
    }
  }
  updateButtonStates();
}

void killPet(String deathReason = "") {
    spiralFill(display, SH110X_WHITE);
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
    display.println("press SELECT to\nrestart.");
    display.display();
    updateButtonStates();
    while(!rightButtonState) {updateButtonStates();}
    esp_restart();
}

void debug() {
  DateTime now = rtc.now();

  String yearStr = String(now.year(), DEC);
  String monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
  String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
  String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC);
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

  String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;

  display.setCursor(0, 0);
  display.println(formattedTime);
  display.print("gyro: X:");
  display.print(String(angleX) + " Y:" + String(angleY) + " Z:" + String(angleZ));
}

bool detectCursorTouch(int startX, int startY, int endX, int endY) {
  if (cursorX > startX && cursorX < startX + endX && cursorY > startY && cursorY < startY + endY) {
    return true;
  } else {
    return false;
  }
}

void drawCursor() {
  // legacy cursor:
  // display.drawLine(cursorX, cursorY, cursorX+5, cursorY+6, SH110X_WHITE);
  // display.drawFastVLine(cursorX, cursorY, 6, SH110X_WHITE);
  // display.drawFastHLine(cursorX, cursorY+6, 5, SH110X_WHITE);
  // display.drawFastVLine(cursorX+2, cursorY+6, 4, SH110X_WHITE);

  // bitmap cursor:
  if (itemBeingPlaced != -1) {
    cursorBitmap = itemBeingPlaced;
  } else {
    cursorBitmap = 14;
    const BitmapInfo& bmp2 = bitmaps[15];
    display.drawBitmap(cursorX - 1, cursorY - 1, bmp2.data, bmp2.width, bmp2.height, SH110X_BLACK);  //cursor mask
  }

  const BitmapInfo& bmp = bitmaps[cursorBitmap];
  display.drawBitmap(cursorX, cursorY, bmp.data, bmp.width, bmp.height, SH110X_WHITE);
}

void drawPet(int petNumber, int drawX, int drawY) {
  switch (petNumber) {
    case 0:
      {
        if (movePet == false) {
          if (petSitTimer > 0) {
            drawBitmapFromList(drawX, drawY, petDir, 27, SH110X_BLACK);
            drawBitmapFromList(drawX, drawY, petDir, 26, SH110X_WHITE);
            petSitTimer--;
          } else {
            drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseStillBigMask, 18, 28, SH110X_BLACK);
            drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseStillBig, 16, 26, SH110X_WHITE);
          }
        } else {
          if (petMoveAnim < 3) {
            drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseWalkMask, 18, 27, SH110X_BLACK);
            drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseWalk, 16, 26, SH110X_WHITE);
          } else {
            drawBitmapWithDirection(drawX, drawY+1, petDir, pet_gooseWalk2Mask, 19, 26, SH110X_BLACK);
            drawBitmapWithDirection(drawX, drawY+1, petDir, pet_gooseWalk2, 17, 25, SH110X_WHITE);
          }
        }
        updateButtonStates();
        if (detectCursorTouch(drawX, drawY, 16, 26) && rightButtonState) {
            showPetMenu = !showPetMenu;
            waitForSelectRelease();
          }
        
        break;
      }
  }
}

void drawBottomBar() {
  display.fillRect(0, 113, 128, 15, SH110X_BLACK);
  display.drawRect(0, 113, 128, 15, SH110X_WHITE);
  display.drawFastVLine(41, 113, 15, SH110X_WHITE);
  display.drawFastVLine(86, 113, 15, SH110X_WHITE);
}

void drawBottomUI() {
  switch (depth) {
    case 0:
      {
        drawBottomBar();
        display.drawBitmap(16, 115, ui_pencil, 11, 11, SH110X_WHITE);
        display.drawBitmap(58, 116, ui_menu, 12, 9, SH110X_WHITE);
        display.drawBitmap(102, 115, ui_settings, 11, 11, SH110X_WHITE);

        if (detectCursorTouch(0, 113, 40, 15)) {  //edit buttton check
          if (rightButtonState) {
            firstOption = 1;
            depth = 1;
            waitForSelectRelease();
          } else {
            display.drawFastVLine(2, 115, 11, SH110X_WHITE);
            display.drawFastVLine(39, 115, 11, SH110X_WHITE);
          }
        }

        if (detectCursorTouch(41, 113, 45, 15)) {  //menu button check
          if (rightButtonState) {
            firstOption = 2;
            depth = 1;
            waitForSelectRelease();
          } else {
            display.drawFastVLine(43, 115, 11, SH110X_WHITE);
            display.drawFastVLine(84, 115, 11, SH110X_WHITE);
          }
        }

        if (detectCursorTouch(87, 113, 41, 15)) {  //settings button check
          if (rightButtonState) {
            firstOption = 3;
            depth = 1;
            waitForSelectRelease();
          } else {
            display.drawFastVLine(88, 115, 11, SH110X_WHITE);
            display.drawFastVLine(125, 115, 11, SH110X_WHITE);
          }
        }
        break;
      }
    case 1: {
        switch (firstOption) {
          case 1:     //edit menu inside
            {
              drawBottomBar();
              display.drawBitmap(16, 115, ui_back, 9, 8, SH110X_WHITE);
              display.drawBitmap(58, 116, ui_inventory, 16, 9, SH110X_WHITE);
              display.drawBitmap(102, 115, ui_shop, 16, 11, SH110X_WHITE);

              if (detectCursorTouch(0, 113, 40, 15)) {  //back buttton check
                if (rightButtonState) {
                  firstOption = 0;
                  depth = 0;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(2, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(39, 115, 11, SH110X_WHITE);
                }
              }

              if (detectCursorTouch(41, 113, 45, 15)) {  //inventory button check
                if (rightButtonState) {
                  secondOption = 1;
                  depth = 2;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(43, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(84, 115, 11, SH110X_WHITE);
                }
              }

              if (detectCursorTouch(86, 113, 41, 15)) {  //shop button check
                if (rightButtonState) {
                  secondOption = 2;
                  thirdOption = 1;
                  depth = 2;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(88, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(125, 115, 11, SH110X_WHITE);
                }
              }
              break;
            }
          case 2:
            {  //main menu inside
              drawBottomBar();
              display.drawBitmap(16, 115, ui_back, 9, 8, SH110X_WHITE);
              display.drawBitmap(58, 116, item_apple, 11, 11, SH110X_WHITE);
              const BitmapInfo& bmp = bitmaps[20];
              display.drawBitmap(102, 115, bmp.data, bmp.width, bmp.height, SH110X_WHITE);

              if (detectCursorTouch(0, 113, 40, 15)) {  //back buttton check
                if (rightButtonState) {
                  firstOption = 0;
                  depth = 0;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(2, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(39, 115, 11, SH110X_WHITE);
                }
              }

              if (detectCursorTouch(41, 113, 45, 15)) {  //apple button check
                if (rightButtonState) {
                  secondOption = 1;
                  depth = 2;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(43, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(84, 115, 11, SH110X_WHITE);
                }
              }

              if (detectCursorTouch(86, 113, 41, 15)) {  //controller button check
                
                if (rightButtonState) {
                  secondOption = 2;
                  depth = 2;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(88, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(125, 115, 11, SH110X_WHITE);
                }
              }
              break;
            }
          case 3: {
          updateButtonStates();
              if (!startHandlingPlacing) {
                drawSettings();
              }
              drawBottomBar();
              display.drawBitmap(16, 115, ui_back, 9, 8, SH110X_WHITE);

              if (detectCursorTouch(0, 113, 40, 15)) {  //back buttton check
                if (rightButtonState) {
                  firstOption = 0;
                  depth = 0;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(2, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(39, 115, 11, SH110X_WHITE);
                }
              }
            break;
          }
        }
        break;
      }
    case 2:
      {
        switch (firstOption) {
          case 1: {  //inventory
            switch (secondOption) {
              case 1: {
                updateButtonStates();
                if (!startHandlingPlacing) {
                  drawInventory();
                }
                drawBottomBar();
                display.drawBitmap(16, 115, ui_back, 9, 8, SH110X_WHITE);

                if (detectCursorTouch(0, 113, 40, 15)) {  //back buttton check
                  if (rightButtonState) {
                    firstOption = 1;
                    depth = 1;
                    waitForSelectRelease();
                  } else {
                    display.drawFastVLine(2, 115, 11, SH110X_WHITE);
                    display.drawFastVLine(39, 115, 11, SH110X_WHITE);
                  }
                }
                break;
              }
              case 2: { //shop
                updateButtonStates();
                if (!startHandlingPlacing) {
                  drawShop();
                }
                drawBottomBar();
                display.drawBitmap(16, 115, ui_back, 9, 8, SH110X_WHITE);

                if (detectCursorTouch(0, 113, 40, 15)) {  //back button check
                  if (rightButtonState) {
                    firstOption = 1;
                    depth = 1;
                    waitForSelectRelease();
                  } else {
                    display.drawFastVLine(2, 115, 11, SH110X_WHITE);
                    display.drawFastVLine(39, 115, 11, SH110X_WHITE);
                  }
                }
                break;
              }
            }
            break;
            }
            case 2: {  
              switch (secondOption) {
                case 1: {   //food inventory
                  updateButtonStates();
                  if (!handleFoodPlacing) {
                    drawFoodInventory();
                  }
                  drawBottomBar();
                  display.drawBitmap(16, 115, ui_back, 9, 8, SH110X_WHITE);

                  if (detectCursorTouch(0, 113, 40, 15)) {  //back button check
                    if (rightButtonState) {
                      firstOption = 2;
                      depth = 1;
                      waitForSelectRelease();
                    } else {
                      display.drawFastVLine(2, 115, 11, SH110X_WHITE);
                      display.drawFastVLine(39, 115, 11, SH110X_WHITE);
                    }
                  }
                  break;
                }
                case 2: {   //game library
                  updateButtonStates();
                  if (!handleFoodPlacing) {
                    drawGameLibrary();
                  }
                  drawBottomBar();
                  display.drawBitmap(16, 115, ui_back, 9, 8, SH110X_WHITE);

                  if (detectCursorTouch(0, 113, 40, 15)) {  //back button check
                    if (rightButtonState) {
                      firstOption = 2;
                      depth = 1;
                      waitForSelectRelease();
                    } else {
                      display.drawFastVLine(2, 115, 11, SH110X_WHITE);
                      display.drawFastVLine(39, 115, 11, SH110X_WHITE);
                    }
                  }
                  break;
                }
              }
              break;
            }
        }
        break;
      }
  }
}

void drawInventory() {
  display.fillRect(0, 64, 128, 64, SH110X_BLACK);
  display.drawRect(0, 64, 128, 64, SH110X_WHITE);
  display.drawFastHLine(0, 113, 128, SH110X_WHITE);
  display.setCursor(2, 66);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("inventory: ");
  display.print(inventoryItems);
  display.print("/8");

  int charWidth = 6;     // Approximate width of one character
  int lineHeight = 8;    // Height of one text line
  int lineSpacing = 10;  // Reduce this for tighter layout
  int itemBoxWidth = 60;
  int maxCharsPerLine = itemBoxWidth / charWidth;

  for (int i = 0; i < inventoryItems; i++) {
    int col = (i > 3) ? 1 : 0;
    int row = (i > 3) ? i - 4 : i;
    int itemX = 4 + col * 64;
    int itemY = 76 + row * lineSpacing;

    String name = displayNames[inventory[i]];
    int lineLen = min((int)name.length(), maxCharsPerLine);

    bool hovered = detectCursorTouch(itemX, itemY, itemBoxWidth, lineHeight * 2);
    display.setTextColor(hovered ? SH110X_BLACK : SH110X_WHITE,
                        hovered ? SH110X_WHITE : SH110X_BLACK);

    if (hovered && rightButtonState) {
      if (itemBeingPlaced == -1) {
        itemBeingPlaced = inventory[i];
        waitForSelectRelease();
        Serial.println("starting placement handling");
        Serial.print("set itemBeingPlaced to: ");
        Serial.println(itemBeingPlaced);
        startHandlingPlacing = true;
      }
    }

    // First line
    display.setCursor(itemX, itemY);
    display.println(name.substring(0, lineLen));

    // Optional second line if needed (check room)
    if (name.length() > maxCharsPerLine && itemY + lineHeight < 127) {
      display.setCursor(itemX, itemY + lineHeight);
      display.println(name.substring(maxCharsPerLine));
    }
  }
}


void drawFoodInventory() {
  display.fillRect(0, 64, 128, 64, SH110X_BLACK);
  display.drawRect(0, 64, 128, 64, SH110X_WHITE);
  display.drawFastHLine(0, 113, 128, SH110X_WHITE);
  display.setCursor(2, 66);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("food: ");
  display.print(foodInventoryItems);
  display.print("/8");

  int charWidth = 6;     // Approximate width of one character
  int lineHeight = 8;    // Height of one text line
  int lineSpacing = 10;  // Reduce this for tighter layout
  int itemBoxWidth = 60;
  int maxCharsPerLine = itemBoxWidth / charWidth;

  for (int i = 0; i < foodInventoryItems; i++) {
    int col = (i > 3) ? 1 : 0;
    int row = (i > 3) ? i - 4 : i;
    int itemX = 4 + col * 64;
    int itemY = 76 + row * lineSpacing;

    String name = displayNames[foodInventory[i]];
    int lineLen = min((int)name.length(), maxCharsPerLine);

    bool hovered = detectCursorTouch(itemX, itemY, itemBoxWidth, lineHeight * 2);
    display.setTextColor(hovered ? SH110X_BLACK : SH110X_WHITE,
                        hovered ? SH110X_WHITE : SH110X_BLACK);

    if (hovered && rightButtonState) {
      if (itemBeingPlaced == -1) {
        itemBeingPlaced = foodInventory[i];
        waitForSelectRelease();
        Serial.println("starting placement handling");
        Serial.print("set itemBeingPlaced to: ");
        Serial.println(itemBeingPlaced);
        handleFoodPlacing = true;
      }
    }

    // First line
    display.setCursor(itemX, itemY);
    display.println(name.substring(0, lineLen));

    // Optional second line if needed (check room)
    if (name.length() > maxCharsPerLine && itemY + lineHeight < 127) {
      display.setCursor(itemX, itemY + lineHeight);
      display.println(name.substring(maxCharsPerLine));
    }
  }
}

void drawEmotionUI() {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  display.print("HUN: ");
  display.print(petHunger);
  if (petHunger < 50) {
    display.println("!");
  } else {
    display.println("");
  }
  display.print("FUN: ");
  display.print(petFun);
  if (petFun < 50) {
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
}

void drawHomeItems() {
  display.drawFastHLine(0, 42, 127, SH110X_WHITE);

  for (int i = 0; i < amountItemsPlaced; i++) {
    const BitmapInfo& bmp = bitmaps[placedHomeItems[i]];
    display.drawBitmap(placedHomeItemsX[i], placedHomeItemsY[i], bmp.data, bmp.width, bmp.height, SH110X_WHITE);
    if (placedHomeItems[i] == 5) {
      display.fillRect(placedHomeItemsX[i] + 6, placedHomeItemsY[i], 3, -27, SH110X_WHITE);
    }
  }

  for (int i = 0; i < amountFoodPlaced; i++) {
    const BitmapInfo& bmp = bitmaps[placedFood[i]];
    display.drawBitmap(placedFoodX[i], placedFoodY[i], bmp.data, bmp.width, bmp.height, SH110X_WHITE);
  }
}

void handleItemPlacing() {
  Serial.println(rightButtonState);
  Serial.println(itemBeingPlaced);

  handleItemsGotThrough++;
  startHandlingPlacing = false;
  int placedCount = amountItemsPlaced;
  addToList(placedHomeItems, placedCount, 30, itemBeingPlaced);
  placedCount = amountItemsPlaced;
  addToList(placedHomeItemsX, placedCount, 30, cursorX);
  placedCount = amountItemsPlaced;
  if (itemBeingPlaced == 5) {
    addToList(placedHomeItemsY, placedCount, 30, 27);
  } else {
    addToList(placedHomeItemsY, placedCount, 30, cursorY);
  }

  Serial.print("Placing item: ");
  Serial.println(itemBeingPlaced);
  Serial.print("At X: ");
  Serial.print(cursorX);
  Serial.print(" Y: ");
  Serial.println(cursorY);

  amountItemsPlaced += 1;
  removeFromList(inventory, inventoryItems, indexOf(inventory, inventoryItems, itemBeingPlaced));
  inventoryItems--;
  Serial.print("Total items placed: ");
  Serial.println(amountItemsPlaced);
  itemBeingPlaced = -1;
  printPlacedHomeItems();
  waitForSelectRelease();
}

void handleFoodPlacingLogic() {
  Serial.println(rightButtonState);
  Serial.println(itemBeingPlaced);

  handleItemsGotThrough++;
  handleFoodPlacing = false;
  int placedCount = amountFoodPlaced;
  addToList(placedFood, placedCount, 10, itemBeingPlaced);
  placedCount = amountFoodPlaced;
  addToList(placedFoodX, placedCount, 10, cursorX);
  placedCount = amountFoodPlaced;
  addToList(placedFoodY, placedCount, 10, cursorY);

  Serial.print("Placing item: ");
  Serial.println(itemBeingPlaced);
  Serial.print("At X: ");
  Serial.print(cursorX);
  Serial.print(" Y: ");
  Serial.println(cursorY);

  amountFoodPlaced += 1;
  removeFromList(foodInventory, foodInventoryItems, indexOf(foodInventory, foodInventoryItems, itemBeingPlaced));
  foodInventoryItems--;
  Serial.print("Total items placed: ");
  Serial.println(amountFoodPlaced);
  itemBeingPlaced = -1;
  waitForSelectRelease();
}

void updateButtonStates() {
  leftButtonState = digitalRead(leftButton) == LOW;
  middleButtonState = digitalRead(middleButton) == LOW;
  rightButtonState = digitalRead(rightButton) == LOW;
  powerSwitchState = digitalRead(SWITCH_PIN) == LOW;
}

void drawPetMenu() {
  int petMenuX = petX + 10;
  int petMenuY = petY + 10;
  display.fillRoundRect(petMenuX, petMenuY, 30, 50, 2, SH110X_WHITE);
  display.drawFastHLine(petMenuX + 1, petMenuY + 10, 28, SH110X_BLACK);
  display.setTextColor(SH110X_BLACK);
  display.setTextSize(1);
  display.setCursor(petMenuX + 1, petMenuY + 1);
  display.print("move");
  display.setTextColor(SH110X_WHITE);
  updateButtonStates();
  if (rightButtonState) {
    if (!detectCursorTouch(petMenuX, petMenuY, 30, 50)) {
      showPetMenu = false;
    } else if (detectCursorTouch(petMenuX, petMenuY, 30, 10)) {
      movingPet = true;
      waitForSelectRelease();
      showPetMenu = false;
    }
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
    }
  }
}

void drawLiveData() {
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  now = rtc.now();
  display.setCursor(55, 0);
  if (liveDataTimer < 100) {
    display.print(now.hour());
    display.print(":");
    if (now.minute() < 10) {
      display.print("0");
      display.print(now.minute());
    } else {
      display.print(now.minute());
    }
  } else {
    display.print(now.day()); display.print("/"); display.print(now.month()); display.print("/"); display.print(now.year());
  }
  
  liveDataTimer++;
  if (liveDataTimer > 200) {
    liveDataTimer = 0; 
  }
  
  display.setTextColor(SH110X_WHITE);
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
}


int convertBatteryPercent(float voltage) {
  if (voltage > 4.2) voltage = 4.2;
  if (voltage < 3.3) voltage = 3.3;
  return (int)((voltage - 3.3) / (4.2 - 3.3) * 100);
}

int averageADC(int pin, int samples = 64) {
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delay(1);
  }
  return total / samples;
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
  
  if (petX == petMoveX && petY == petMoveY) {
    movePet = false;
  }
  petMoveAnim++;
  if (petMoveAnim > 4) {
    petMoveAnim = 0;
  }
}

void drawGameLibrary() {
  display.fillRect(0, 64, 128, 64, SH110X_BLACK);
  display.drawRect(0, 64, 128, 64, SH110X_WHITE);
  display.drawFastHLine(0, 113, 128, SH110X_WHITE);
  display.setCursor(2, 66);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("games: ");
  display.print(gameLibraryCount);
  display.print("/2");

  int charWidth = 6;     // Approximate width of one character
  int lineHeight = 8;    // Height of one text line
  int lineSpacing = 10;  // Reduce this for tighter layout
  int itemBoxWidth = 60;
  int maxCharsPerLine = itemBoxWidth / charWidth;

  for (int i = 0; i < gameLibraryCount; i++) {
    int col = (i > 3) ? 1 : 0;
    int row = (i > 3) ? i - 4 : i;
    int itemX = 4 + col * 64;
    int itemY = 76 + row * lineSpacing;

    String name = gameNames[gameLibrary[i]];
    int lineLen = min((int)name.length(), maxCharsPerLine);

    bool hovered = detectCursorTouch(itemX, itemY, itemBoxWidth, lineHeight);
    display.setTextColor(hovered ? SH110X_BLACK : SH110X_WHITE,
                        hovered ? SH110X_WHITE : SH110X_BLACK);

    if (hovered && rightButtonState) {
      if (name == "pong") {
        drawCheckerboard(petPongLVL + 1);
        pong();
      } else {
        drawCheckerboard(petVeridiumLVL + 1);
        veridium();
      }
      waitForSelectRelease();
    }

    // First line
    display.setCursor(itemX, itemY);
    display.println(name.substring(0, lineLen));

    // Optional second line if needed (check room)
    if (name.length() > maxCharsPerLine && itemY + lineHeight < 127) {
      display.setCursor(itemX, itemY + lineHeight);
      display.println(name.substring(maxCharsPerLine));
    }
  }
}

void drawShop() {
  uiTimer = 100;
  display.fillRect(0, 0, 127, 112, SH110X_BLACK);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("shop  $");
  display.print(money);
  display.drawFastHLine(0, 8, 127, SH110X_WHITE);
  
  updateButtonStates();

  if (detectCursorTouch(1, 10, 72, 8) && rightButtonState) {
    thirdOption = 1;
  } else if (detectCursorTouch(78, 10, 24, 8) && rightButtonState) {
    thirdOption = 2;
  }

  int* currentShopItems;
  float* currentShopPrices;
  int* currentInventory;
  //int* currentInventoryItems;
  int currentShopLength;

  if (thirdOption == 1) { //construction tab
    display.setCursor(1, 10);
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print("construction");
    display.setTextColor(SH110X_WHITE);
    display.print(" food");
    currentShopItems = constructionShopItems;
    currentShopPrices = constructionShopPrices;
    currentShopLength = constructionShopLength;
    currentInventory = inventory;
    //currentInventoryItems = &inventoryItems;
  } else { //food tab
    display.setCursor(1, 10);
    display.setTextColor(SH110X_WHITE);
    display.print("construction ");
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print("food");
    currentShopItems = foodShopItems;
    currentShopPrices = foodShopPrices;
    currentShopLength = foodShopLength;
    currentInventory = foodInventory;
    //currentInventoryItems = &foodInventoryItems;
  }

  display.setCursor(0, 20);

  for (int i = 0; i < currentShopLength; i++) {
    updateButtonStates();

    if (detectCursorTouch(0, i * 8 + 20, 127, 8)) {
      display.setTextColor(SH110X_BLACK, SH110X_WHITE);
      if (rightButtonState) {
        if (money >= currentShopPrices[i]) {
          money -= currentShopPrices[i];

          int currentInventoryItems;

          if (thirdOption == 1) {
            currentInventoryItems = inventoryItems;
            addToList(inventory, inventoryItems, 8, currentShopItems[i]);
          } else {
            currentInventoryItems = foodInventoryItems;
            addToList(foodInventory, foodInventoryItems, 8, currentShopItems[i]);
          }

          //addToList(currentInventory, currentInventoryItems, 10, currentShopItems[i]);
      
          waitForSelectRelease();
        }
      }
    } else {
      display.setTextColor(SH110X_WHITE);
    }
    display.print(displayNames[currentShopItems[i]]);
    display.print(" - $");
    display.println(currentShopPrices[i]);
  }

}

bool drawCenteredButton(String label, int y) {
  int16_t x1, y1;
  uint16_t w, h;

  // Get the size of the text
  display.getTextBounds(label, 0, y, &x1, &y1, &w, &h);

  // Calculate the centered X position
  int16_t x = (display.width() - w) / 2;

  // Draw the button (background box and text)
  display.drawRect(x - 2, y - 2, w + 4, h + 4, SH110X_WHITE);
  display.setCursor(x, y);
  

  updateButtonStates();  // You probably need this before reading touch state
  bool buttonClicked = false;
  // Handle touch
  if (detectCursorTouch(x - 2, y - 2, w + 4, h + 4)) {
    display.setTextColor(SH110X_BLACK, SH110X_WHITE); // optional inverse
    if (rightButtonState) {
      waitForSelectRelease();  // Now this runs properly
      buttonClicked = true;
    }
  }

  display.print(label);
  display.setTextColor(SH110X_WHITE);

  if (buttonClicked) {
    return true;
  } else {
    return false;
  }
}

void drawSettings() {
  uiTimer = 100;
  display.fillRect(0, 0, 127, 112, SH110X_BLACK);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("settings");
  display.drawFastHLine(0, 8, 127, SH110X_WHITE);
  display.setCursor(0, 10);
  updateButtonStates();

  switch (settingsOption) {
    case 0: {    
      if (detectCursorTouch(0, 10, 102, 8)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          settingsOption = 1;
        }
      }

      display.println("set date and time");

      display.setTextColor(SH110X_WHITE);

      if (detectCursorTouch(0, 18, 102, 8)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          settingsOption = 2;
        }
      }

      display.println("gyro sensitivity");

      display.setTextColor(SH110X_WHITE);

      if (detectCursorTouch(0, 26, 102, 8)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          settingsOption = 3;if (detectCursorTouch(0, 34, 102, 8)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          settingsOption = 4;
        }
      }

      display.println("save manager");

      display.setTextColor(SH110X_WHITE);
      break;
        }
      }

      display.println("loop delay");

      display.setTextColor(SH110X_WHITE);

      if (detectCursorTouch(0, 34, 102, 8)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          settingsOption = 4;
        }
      }

      display.println("save manager");

      display.setTextColor(SH110X_WHITE);

      display.setCursor(0, 100);

      if (detectCursorTouch(0, 100, 102, 8)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          esp_restart();
        }
      }

      display.println("restart");

      display.setTextColor(SH110X_WHITE);
      break;
    }
    case 1: {
      static bool dateTimeInitialized = false;
      static int year, month, day, hour, minute;

      if (!dateTimeInitialized) {
        DateTime now = rtc.now(); // Get current RTC time once
        year = now.year();
        month = now.month();
        day = now.day();
        hour = now.hour();
        minute = now.minute();
        dateTimeInitialized = true;
      }

      display.setCursor(0, 10);
      display.print("edit date & time");

      // Field positions
      int fieldX[] = {0, 40, 60, 80, 100};
      int fieldY = 40;

      int *values[] = {&year, &month, &day, &hour, &minute};
      String labels[] = {
        String(year), String(month), String(day),
        String(hour), String(minute)
      };

      for (int i = 0; i < 5; i++) {
        int x = fieldX[i];

        // Draw + button
        display.setCursor(x, fieldY - 10);
        if (detectCursorTouch(x, fieldY - 10, 20, 10)) {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
          if (rightButtonState) {
            *values[i] += 1;
            if (i == 1 && *values[i] > 12) *values[i] = 1; // Month max
            if (i == 2 && *values[i] > 31) *values[i] = 1; // Day max
            if (i == 3 && *values[i] > 23) *values[i] = 0; // Hour max
            if (i == 4 && *values[i] > 59) *values[i] = 0; // Minute max
            waitForSelectRelease();
          }
        } else {
          display.setTextColor(SH110X_WHITE);
        }
        display.print("+");

        // Draw value
        display.setCursor(x, fieldY);
        if (selectedField == i) {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        } else {
          display.setTextColor(SH110X_WHITE);
        }
        display.print(labels[i]);

        // Draw - button
        display.setCursor(x, fieldY + 10);
        if (detectCursorTouch(x, fieldY + 10, 20, 10)) {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
          if (rightButtonState) {
            *values[i] -= 1;
            if (i == 1 && *values[i] < 1) *values[i] = 12;
            if (i == 2 && *values[i] < 1) *values[i] = 31;
            if (i == 3 && *values[i] < 0) *values[i] = 23;
            if (i == 4 && *values[i] < 0) *values[i] = 59;
            waitForSelectRelease();
          }
        } else {
          display.setTextColor(SH110X_WHITE);
        }
        display.print("-");
      }

      // Confirm button
      bool confirmPressed = detectCursorTouch(30, 80, 60, 10);
      display.setCursor(30, 80);
      if (confirmPressed) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          rtc.adjust(DateTime(year, month, day, hour, minute, 0));
          settingsOption = 0;
          selectedField = 0;
          dateTimeInitialized = false;
        }
      } else {
        display.setTextColor(SH110X_WHITE);
      }
      display.print("confirm");

      updateButtonStates();
      break;
    }
    case 2: {
      static int sensitivityX = gyroSensitivityX;
      static int sensitivityY = gyroSensitivityY;
      static int sensitivityZ = gyroSensitivityZ;

      display.setCursor(0, 10);
      display.print("gyro sensitivity");

      String labels[] = {"X:", "Y:", "Z:"};
      int *values[] = {&sensitivityX, &sensitivityY, &sensitivityZ};
      int startY = 30;

      for (int i = 0; i < 3; i++) {
        int y = startY + i * 20;

        // Label
        display.setCursor(10, y);
        display.setTextColor(SH110X_WHITE);
        display.print(labels[i]);

        // + button
        display.setCursor(40, y);
        if (detectCursorTouch(40, y, 10, 10)) {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
          if (rightButtonState) {
            (*values[i])++;
            if (*values[i] > 100) *values[i] = 100; // Optional upper limit
            waitForSelectRelease();
          }
        } else {
          display.setTextColor(SH110X_WHITE);
        }
        display.print("+");

        // Value
        display.setCursor(60, y);
        display.setTextColor(SH110X_WHITE);
        display.print(*values[i]);

        // - button
        display.setCursor(90, y);
        if (detectCursorTouch(90, y, 10, 10)) {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
          if (rightButtonState) {
            (*values[i])--;
            if (*values[i] < 0) *values[i] = 0; // Optional lower limit
            waitForSelectRelease();
          }
        } else {
          display.setTextColor(SH110X_WHITE);
        }
        display.print("-");
      }

      // Confirm button
      bool confirmPressed = detectCursorTouch(30, 100, 60, 10);
      display.setCursor(30, 100);
      if (confirmPressed) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          gyroSensitivityX = sensitivityX;
          gyroSensitivityY = sensitivityY;
          gyroSensitivityZ = sensitivityZ;
          settingsOption = 0;
          waitForSelectRelease();
        }
      } else {
        display.setTextColor(SH110X_WHITE);
      }
      display.print("confirm");

      updateButtonStates();
      break;
    }
    case 3: {
      static int delayTemp = loopDelay;     

      display.setCursor(0, 10);
      display.print("loop delay");

      String labels[] = {"delay (ms):"};
      int *values[] = {&delayTemp};
      int startY = 50;

      for (int i = 0; i < 1; i++) {
        int y = startY + i * 20;

        // Label
        display.setCursor(0, y);
        display.setTextColor(SH110X_WHITE);
        display.print(labels[i]);

        // + button
        display.setCursor(60, y);
        if (detectCursorTouch(60, y, 10, 10)) {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
          if (rightButtonState) {
            (*values[i])++;
            if (*values[i] > 100) *values[i] = 100; // Optional upper limit
            waitForSelectRelease();
          }
        } else {
          display.setTextColor(SH110X_WHITE);
        }
        display.print("+");

        // Value
        display.setCursor(80, y);
        display.setTextColor(SH110X_WHITE);
        display.print(*values[i]);

        // - button
        display.setCursor(110, y);
        if (detectCursorTouch(110, y, 10, 10)) {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
          if (rightButtonState) {
            (*values[i])--;
            if (*values[i] < 0) *values[i] = 0; // Optional lower limit
            waitForSelectRelease();
          }
        } else {
          display.setTextColor(SH110X_WHITE);
        }
        display.print("-");
      }

      // Confirm button
      bool confirmPressed = detectCursorTouch(30, 100, 60, 10);
      display.setCursor(30, 100);
      if (confirmPressed) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          loopDelay = delayTemp;
          settingsOption = 0;
          waitForSelectRelease();
        }
      } else {
        display.setTextColor(SH110X_WHITE);
      }
      display.print("confirm");

      updateButtonStates();
      break;
    }
    case 4: {
      drawCenteredText(display, "save manager", 10);

      if (drawCenteredButton("save to eeprom", 22)) {
        saveGameToEEPROM();
      }

      if (drawCenteredButton("save to ram", 32)) {
        saveGameToRam();
      }

      if (drawCenteredButton("load from eeprom", 42)) {
        loadGameFromEEPROM();
      }

      if (drawCenteredButton("load from ram", 52)) {
        loadGameFromRam();
      }

      drawCenteredText(display, "save interval (mins):", 65);

      // + button
      display.setCursor(60, 76);
      if (detectCursorTouch(60, 76, 10, 10)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          saveInterval++;
          waitForSelectRelease();
        }
      } else {
        display.setTextColor(SH110X_WHITE);
      }
      display.print("+");

      // Value
      display.setCursor(80, 76);
      display.setTextColor(SH110X_WHITE);
      display.print(saveInterval);

      // - button
      display.setCursor(110, 76);
      if (detectCursorTouch(110, 76, 10, 10)) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        if (rightButtonState) {
          saveInterval--;
          if (saveInterval < 0) saveInterval = 0; 
          waitForSelectRelease();
        }
      } else {
        display.setTextColor(SH110X_WHITE);
      }
      display.print("-");

      if (drawCenteredButton("exit", 90)) {
        settingsOption = 0;
      }

      break;
    }
  }
  
}



void updateGyro() {
  mpu.gyroUpdate();
  mpu.accelUpdate();

  int gyroX = round(mpu.gyroX() + gyroXOffset) * gyroSensitivityX * -1;  //multiply gyro values by user set sensitivity. x value is inverted since gyro is upside down in hardware 
  int gyroY = round(mpu.gyroY() + gyroYOffset) * gyroSensitivityY;
  int gyroZ = round(mpu.gyroZ() + gyroZOffset) * gyroSensitivityZ;

  int accelX = round(mpu.accelX() + accelXOffset);
  int accelY = round(mpu.accelY() + accelYOffset);
  int accelZ = round(mpu.accelZ() + accelZOffset);

  // Serial.printf("X: %f, Y: %f, Z: %f\n", accelX, accelY, accelZ);

  totalG = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  unsigned long updateNow = millis();
  float deltaTime = (updateNow - lastUpdate) / 1000.0;
  lastUpdate = updateNow;

  angleX += gyroX * deltaTime;
  angleY += gyroY * deltaTime;
  angleZ += gyroZ * deltaTime;

  posX += gyroX * deltaTime;
  posY += gyroY * deltaTime;
  posZ += gyroZ * deltaTime;
  // Serial.printf("TOTAL X: %f, Y: %f, Z: %f\n", posX, posY, posZ);
  // Serial.printf("total gforce: %f\n", totalG);
}



void runSaveInterval() {
  DateTime now = rtc.now();

  TimeSpan elapsed = now - lastRunTime;
  if (elapsed.totalseconds() >= saveInterval * 60) {
    
    Serial.println("interval passed, saving game");

    saveGameToEEPROM();

    lastRunTime = now;
  }
}
//BEHAVIOUR TREE STUFF (PRETTY SIGMA)
DRAM_ATTR enum NodeStatus { SUCCESS,
                            FAILURE,
                            RUNNING };

DRAM_ATTR class Node {
public:
  virtual NodeStatus tick() = 0;
};

DRAM_ATTR class Selector : public Node {
  std::vector<Node*> children;
public:
  Selector(std::initializer_list<Node*> nodes)
    : children(nodes) {}
  NodeStatus tick() override {
    for (auto child : children) {
      NodeStatus status = child->tick();
      if (status != FAILURE) return status;
    }
    return FAILURE;
  }
};

DRAM_ATTR class Sequence : public Node {
  std::vector<Node*> children;
public:
  Sequence(std::initializer_list<Node*> nodes)
    : children(nodes) {}
  NodeStatus tick() override {
    for (auto child : children) {
      NodeStatus status = child->tick();
      if (status != SUCCESS) return status;
    }
    return SUCCESS;
  }
};


//LEAF NODES
class IsHungry : public Node {
public:
  NodeStatus tick() override {
    return (petHunger < 50) ? SUCCESS : FAILURE;
  }
};

class IsBored : public Node {
public:
  NodeStatus tick() override {
    return (petFun < 50) ? SUCCESS : FAILURE;
  }
};

class IsTired : public Node {
public:
  NodeStatus tick() override {
    return (petSleep < 30) ? SUCCESS : FAILURE;
  }
};

class IsBeingShaken : public Node {
public:
  NodeStatus tick() override {
    return (totalG > 2) ? SUCCESS : FAILURE;
  }
};

class AskForFood : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime >= messageMaxTime) {
    if (random(0, 50) == 1) {
      int messageRandomiser = random(0, hungryLinesCount);
      petMessage(hungryLines[messageRandomiser]);
      return SUCCESS;
    } else {
      return RUNNING;
    }
    } else {
      return RUNNING;
    }
    
  } 
};

class AskForPlay : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime >= messageMaxTime) {
    if (random(0, 50) == 1) {
      int messageRandomiser = random(0, boredLinesCount);
      petMessage(boredLines[messageRandomiser]);
      return SUCCESS;
    } else {
      return RUNNING;
    }
    } else {
      return RUNNING;
    }
    
  } 
};

class AskForSleep : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime >= messageMaxTime) {
    if (random(0, 50) == 1) {
      int messageRandomiser = random(0, tiredLinesCount);
      petMessage(tiredLines[messageRandomiser]);
      return SUCCESS;
    } else {
      return RUNNING;
    }
    } else {
      return RUNNING;
    }
    
  } 
};

class Idle : public Node {
public:
  NodeStatus tick() override {
    //0.8% chance to yap
    if (messageDisplayTime >= messageMaxTime) {
    if (random(0, 200) == 1) {
      int messageRandomiser = random(0, idleLinesCount);
      petMessage(idleLines[messageRandomiser]);
    } else if (random(0, 200) == 2) {
      petMessage(generateSentence());
    }

    
    }
    if (movePet == false) {
      if (random(0, 100) == 1) {
        startMovingPet(random(0, 105), random(35, 100), 1);
      } else if (random(0, 200) == 2) {
        int index = indexOf(placedHomeItems, amountItemsPlaced, 3);
        if (index != -1) {
          int itemX = placedHomeItemsX[index];
          int itemY = placedHomeItemsY[index];
          if (petX != itemX || petY != itemY) {
          startMovingPet(itemX, itemY, 2);
          } else {
            if (petSitTimer < 5) {
              sitPet(1000);
            }
          }
        }
      }
    }
    return SUCCESS;
  }
};

class IsFoodAvailable : public Node {
public:
  NodeStatus tick() override {

    if (amountFoodPlaced > 0) {
      return SUCCESS;
    } else {
      return FAILURE;
    }
  }
};

class BeingCarried : public Node {
public:
  NodeStatus tick() override {

    if (movingPet) {
      return SUCCESS;
    } else {
      return FAILURE;
    }
  }
};

class ComplainAboutBeingCarried : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime >= messageMaxTime) {
      int messageRandomiser = random(0, beingCarriedLinesCount);
      petMessage(beingCarriedLines[messageRandomiser]);
    }

    return SUCCESS;
  }
};

class ComplainAboutBeingShaken : public Node {
public:
  NodeStatus tick() override {

    if (messageDisplayTime >= messageMaxTime) {
      int messageRandomiser = random(0, shakenLinesCount);
      petMessage(shakenLines[messageRandomiser]);
    }

    return SUCCESS;
  }
};

class EatFood : public Node {
public:
  NodeStatus tick() override {

    int lastFoodIndex = amountFoodPlaced - 1;    
    int lastFoodX = placedFoodX[lastFoodIndex];
    int lastFoodY = placedFoodY[lastFoodIndex] - 8;

    if (lastFoodX == petX && lastFoodY == petY) {
      removeFromList(placedFood, lastFoodIndex, lastFoodIndex);
      removeFromList(placedFoodX, lastFoodIndex, lastFoodIndex);
      removeFromList(placedFoodY, lastFoodIndex, lastFoodIndex);
      amountFoodPlaced--;
      petHunger += 30;
      movePet = false;
      petMessage("yum!");
      return SUCCESS;
    } else {
      if (!movePet) {
        startMovingPet(lastFoodX, lastFoodY, 2);
      }
      return RUNNING;
    }
  }
};

class ShouldDie : public Node {
public:
  NodeStatus tick() override {

    if (petHunger < 1 || petSleep < 1) {
      return SUCCESS;
    } else {
      return FAILURE;
    }
  }
};

class Die : public Node {
public:
  NodeStatus tick() override {
    if (petHunger < 1) {
      killPet("starved to death");
    } else {
      killPet("collapsed due to\nexhaustion");
    }
  }
};

DRAM_ATTR Node* tree = new Selector({ new Sequence({ new ShouldDie(), new Die() }),
                                      new Sequence({ new IsBeingShaken(), new ComplainAboutBeingShaken() }),
                                      new Sequence({ new BeingCarried(), new ComplainAboutBeingCarried() }),
                                      new Sequence({ new IsFoodAvailable(), new EatFood() }),
                                      new Sequence({ new IsHungry(), new AskForFood() }),
                                      new Sequence({ new IsTired(), new AskForSleep() }),
                                      new Sequence({ new IsBored(), new AskForPlay() }),
                                      new Idle() });

void loop() {
  if (totalG > 11) {   //kinda funny but annoying
    killPet("got shaken to death"); 
  }
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;     //interval passed
    
    if (movePet) {
      updatePetMovement();
    }
  }
  tree->tick();  //behaviour tree update

  DateTime now = rtc.now();

  //Serial.println(now.second());
  updatePetNeeds();
  rgb.setPixelColor(0, rgb.Color(angleX, angleY, angleZ));  // set led color to angle of device
  rgb.show();

  updateButtonStates();

  display.clearDisplay();

  if (middleButtonState) {
    uiTimer = 100;
  }

  updateGyro();

  if (!leftButtonState) {
    angleX = 0;
    angleY = 0;
    angleZ = 0;
    if (cursorTimer > 0) {
      shouldDrawCursor = true;
      cursorTimer = cursorTimer - 0.1;
    } else {
      shouldDrawCursor = false;
    }
  } else {
    cursorX = constrain(angleX + 64, 0, 126);
    if (itemBeingPlaced == 5) {
      cursorY = 27;
    } else {
      cursorY = constrain(angleY + 64, 0, 126);
    }
    shouldDrawCursor = true;
    cursorTimer = 4;  //how long cursor is displayed after releasing button, 1 = 50ms, 4 = 200ms, so on
  }
  drawHomeItems();
  Serial.println("drawn home items");
  if (movingPet) {
    petX = cursorX;
    petY = cursorY;
    constrain(petY, 43, 127);
    updateButtonStates();
    if (rightButtonState) {
      movingPet = false;
    }
  }

  drawPet(0, petX, petY);

  if (messageDisplayTime < messageMaxTime) {
    drawPetMessage();
  }

  updateButtonStates();

  if (itemBeingPlaced != -1 && rightButtonState && startHandlingPlacing) {
    handleItemPlacing();
  }

  if (itemBeingPlaced != -1 && rightButtonState && handleFoodPlacing) {
    handleFoodPlacingLogic();
  }

  if (uiTimer > 0) {
    if (detectCursorTouch(0, 113, 128, 15)) {
      uiTimer = 100;
    }
    uiTimer--; 
    drawEmotionUI();

    drawLiveData();

    drawBottomUI();
  }

  if (showPetMenu) {
    drawPetMenu();
  }

  if (shouldDrawCursor) {
    drawCursor();
  }

  display.display();

  if (powerSwitchState) {
    Serial.println("going into light sleep, see ya later!");
    display.clearDisplay();
    display.setCursor(12, 10);
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.print("goodnight");
    display.setTextSize(1);
    display.setCursor(15, 50);
    display.print("going to sleep...");
    const BitmapInfo& bmp = bitmaps[17];
    display.drawBitmap(55, 82, bmp.data, bmp.width, bmp.height, SH110X_WHITE);
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
    rgb.setPixelColor(0, rgb.Color(0, 0, 0));
    rgb.show();
    mpu9250_sleep();
    DateTime timeWhenSlept = rtc.now();
    esp_deep_sleep_enable_gpio_wakeup(1 << SWITCH_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH); // enable waking up from deep sleep when switch is turned on / pulled high absoulute nerd

    esp_light_sleep_start();  //yoo he said the thing

    mpu9250_wake();
    DateTime now = rtc.now();

    TimeSpan timeSinceSlept = now - timeWhenSlept;

    int32_t seconds = timeSinceSlept.totalseconds();
    int32_t minutesSinceSlept = seconds / 60;

    petSleep += minutesSinceSlept;
    
    petHunger -= minutesSinceSlept / 10;
    
    petSleep = constrain(petSleep, 0, 120);

    display.clearDisplay();
    display.setCursor(40, 10);
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.print("good");
    display.setCursor(25, 30);
    display.print("morning");
    display.setTextSize(1);
    display.setCursor(17, 70);
    display.print("my SLP is at ");
    display.print(petSleep);
    display.setCursor(17, 80);
    display.print("my HUN is at ");
    display.print(petHunger);
    const BitmapInfo& bmp2 = bitmaps[1];
    display.drawBitmap(55, 90, bmp2.data, bmp2.width, bmp2.height, SH110X_WHITE);
    display.display();
    if (minutesSinceSlept >= saveInterval) {
      saveGameToEEPROM(false);
    }
    delay(1000);
  }

  delay(loopDelay);
}