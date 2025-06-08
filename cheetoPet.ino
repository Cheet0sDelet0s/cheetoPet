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

#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 128  // OLED display height, in pixels
#define OLED_RESET -1      // can set an oled reset pin if desired

/*FLASH, DRAM AND IRAM
anything that is read/written to infrequently should go in flash. (like storage of computer)
anything that is used quite a bit goes in dram (like ram of computer)
anything that is used every iteration very frequently should go in iram (like l1 cache of computer).
this makes things faster and stops the flash of the chip wearing out after lots of use.
try to upload code as little as possible to prolong lifetime of the chip,
it can spontaneously die if you arent carefull.
*/

const int leftButton = 5;
const int middleButton = 6;
const int rightButton = 7;

DRAM_ATTR unsigned long previousMillis = 0;
const long interval = 50;  

DRAM_ATTR DateTime now;

DRAM_ATTR int batteryPercentage = 50;
DRAM_ATTR float batteryVoltage = 3.7;
//buttons
DRAM_ATTR bool leftButtonState = false;
DRAM_ATTR bool middleButtonState = false;
DRAM_ATTR bool rightButtonState = false;
DRAM_ATTR bool powerSwitchState = false;

//menu system
DRAM_ATTR int firstOption = 0;
DRAM_ATTR int secondOption = 0;
DRAM_ATTR int depth = 0;

//debug
DRAM_ATTR int handleItemsGotThrough = 0;

RTC_DS3231 rtc;
MPU9250_asukiaaa mpu(0x69);

DRAM_ATTR char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
DRAM_ATTR int lastSecond = -1;

DRAM_ATTR int liveDataTimer = 0;

//item inventory
DRAM_ATTR int inventory[8] = { 3, 5, 4, 6, 19 };
DRAM_ATTR int inventoryItems = 5;
DRAM_ATTR int placedHomeItems[10] = { 7 };
DRAM_ATTR int amountItemsPlaced = 1;
DRAM_ATTR int placedHomeItemsX[10] = { 98 };
DRAM_ATTR int placedHomeItemsY[10] = { 4 };
DRAM_ATTR int itemBeingPlaced = -1;
DRAM_ATTR bool startHandlingPlacing = false;

//food inventory
DRAM_ATTR int foodInventory[8] = { 18, 16, 18, 16 };
DRAM_ATTR int foodInventoryItems = 4;
DRAM_ATTR int placedFood[10] = {};
DRAM_ATTR int amountFoodPlaced = 0;
DRAM_ATTR int placedFoodX[10] = {};
DRAM_ATTR int placedFoodY[10] = {};
DRAM_ATTR bool handleFoodPlacing = false;

Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

DRAM_ATTR float gyroXOffset = -0.40;
DRAM_ATTR float gyroYOffset = 3;
DRAM_ATTR float gyroZOffset = 1.5;

DRAM_ATTR float angleX = 0;
DRAM_ATTR float angleY = 0;
DRAM_ATTR float angleZ = 0;

//cursor stuff
DRAM_ATTR int cursorX = 500;
DRAM_ATTR int cursorY = 500;
DRAM_ATTR bool shouldDrawCursor = false;
DRAM_ATTR float cursorTimer = 0;
DRAM_ATTR int cursorBitmap = 14;

DRAM_ATTR int uiTimer = 100;

DRAM_ATTR int petHunger = 49;
DRAM_ATTR int petFun = 50;
DRAM_ATTR int petSleep = 50;
DRAM_ATTR int petX = 64;
DRAM_ATTR int petY = 32;
DRAM_ATTR bool showPetMenu = false;
DRAM_ATTR bool movingPet = false;
DRAM_ATTR float money = 20;

DRAM_ATTR int petMoveX = 64;
DRAM_ATTR int petMoveY = 32;
DRAM_ATTR int petMoveSpeed = 0;
DRAM_ATTR bool movePet = false;

DRAM_ATTR unsigned long lastUpdate = 0;

DRAM_ATTR String currentPetMessage;
DRAM_ATTR int messageDisplayTime = 0;
DRAM_ATTR int messageMaxTime = 0;

void petMessage(String message) {
  currentPetMessage = message;
  messageDisplayTime = 0;
  messageMaxTime = (currentPetMessage.length() * 20) / 1.5;
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

//bitmaps and voice lines are in made external in bitmaps.h, defined in bitmaps.cpp

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
  display.print("loading modules");
  display.display();
  delay(500);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    // Set the RTC to the current date & time
    rtc.adjust(DateTime(2025, 6, 6, 7, 53, 0));
  }

  mpu.setWire(&Wire);
  mpu.beginGyro();

  rgb.begin();            // Initialize
  rgb.setBrightness(50);  // Optional: reduce brightness
  rgb.show();             // Initialize all pixels to 'off'

  setBatteryLevel();

  lastUpdate = millis();
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
        display.drawBitmap(drawX, drawY, pet_gooseStillBigMask, 17, 26, SH110X_BLACK);
        display.drawBitmap(drawX, drawY, pet_gooseStillBig, 16, 26, SH110X_WHITE);
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

        if (detectCursorTouch(86, 113, 41, 15)) {  //settings button check
          display.drawFastVLine(88, 115, 11, SH110X_WHITE);
          display.drawFastVLine(125, 115, 11, SH110X_WHITE);
        }
        break;
      }
    case 1:
      {
        switch (firstOption) {
          case 1:
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
                display.drawFastVLine(88, 115, 11, SH110X_WHITE);
                display.drawFastVLine(125, 115, 11, SH110X_WHITE);
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
                  secondOption = 2;
                  depth = 2;
                  waitForSelectRelease();
                } else {
                  display.drawFastVLine(43, 115, 11, SH110X_WHITE);
                  display.drawFastVLine(84, 115, 11, SH110X_WHITE);
                }
              }

              if (detectCursorTouch(86, 113, 41, 15)) {  //shop button check
                display.drawFastVLine(88, 115, 11, SH110X_WHITE);
                display.drawFastVLine(125, 115, 11, SH110X_WHITE);
              }
            }
        }
        break;
      }
    case 2:
      {
        switch (secondOption) {
          case 1:
            {  //inventory
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
          case 2:
            {  //feed menu
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
  display.println(petHunger);
  display.print("FUN: ");
  display.println(petFun);
  display.print("SLP: ");
  display.println(petSleep);
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
  addToList(placedHomeItems, placedCount, 10, itemBeingPlaced);
  placedCount = amountItemsPlaced;
  addToList(placedHomeItemsX, placedCount, 10, cursorX);
  placedCount = amountItemsPlaced;
  if (itemBeingPlaced == 5) {
    addToList(placedHomeItemsY, placedCount, 10, 27);
  } else {
    addToList(placedHomeItemsY, placedCount, 10, cursorY);
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
      petSleep -= 1;
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
    //setBatteryLevel();
  }
  
  display.setTextColor(SH110X_WHITE);
}

// void drawPetMessage() {
//   int bubbleX;
//   int bubbleDirection;

//   if (petX < 64) {
//     bubbleX = petX + 17;
//     bubbleDirection = 1;
//   } else {
//     bubbleX = petX;
//     bubbleDirection = -1;
//   }

//   int bubbleY = petY - 3;
//   int messageLength = currentPetMessage.length();

//   if (bubbleDirection == 1) {
//     display.setCursor(bubbleX + 2, bubbleY - 13);
//   } else {
//     display.setCursor(bubbleX - (2 + messageLength * 6), bubbleY - 13);
//   }

//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE, SH110X_BLACK);
//   display.print(currentPetMessage);
//   display.setTextColor(SH110X_WHITE);

//   display.drawFastVLine(bubbleX, bubbleY, -14, SH110X_WHITE);
//   if (bubbleDirection == -1) {
//     display.drawFastVLine(bubbleX + ((messageLength * 6 + 3) * bubbleDirection), bubbleY - 15, 11, SH110X_WHITE);
//   } else {
//     display.drawFastVLine(bubbleX + ((messageLength * 6 + 2) * bubbleDirection), bubbleY - 15, 11, SH110X_WHITE);
//   }
//   //jesus christ what are these goofy calculations
//   display.drawFastHLine(bubbleX, bubbleY - 15, (messageLength * 6 + 2) * bubbleDirection, SH110X_WHITE);
//   display.drawFastHLine(bubbleX + 5 * bubbleDirection, bubbleY - 5, (messageLength * 6 - 3) * bubbleDirection, SH110X_WHITE);
//   display.drawLine(bubbleX, bubbleY, bubbleX + 5 * bubbleDirection, bubbleY - 5, SH110X_WHITE);


//   messageDisplayTime++;
// }

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

// Left vertical line (adjusted -1 pixel in height)
display.drawFastVLine(edgeX, bubbleY, -bubbleHeight + 1, SH110X_WHITE);

// Right vertical line (adjusted -1 pixel in height)
display.drawFastVLine(edgeX + edgeW, bubbleY - bubbleHeight + 1, bubbleHeight - 4, SH110X_WHITE);

// Top horizontal line (adjusted -1 pixel in length)
display.drawFastHLine(edgeX, bubbleY - bubbleHeight, edgeW - bubbleDirection, SH110X_WHITE);

// Bottom portion of bubble (unchanged)
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


void setBatteryLevel() {
  int raw = averageADC(3, 64);
  float voltage = (raw / 4095) * REF_VOLTAGE;
  batteryVoltage = voltage * 2; // Because of the voltage divider

  batteryPercentage = convertBatteryPercent(batteryVoltage);
  Serial.println(raw);
}

void startMovingPet(int x, int y, int speed) {
  petMoveX = x;
  petMoveY = y;
  petMoveSpeed = speed;
  movePet = true;
}

void updatePetMovement() {
  int xDiff = petMoveX - petX;
  xDiff = xDiff / abs(xDiff);

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

class Idle : public Node {
public:
  NodeStatus tick() override {
    //0.8% chance to yap
    if (messageDisplayTime >= messageMaxTime) {
    if (random(0, 200) == 1) {
      int messageRandomiser = random(0, idleLinesCount);
      petMessage(idleLines[messageRandomiser]);
    }
    return SUCCESS;
  } else {
    return RUNNING;
  }
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
      startMovingPet(lastFoodX, lastFoodY, 2);
      return RUNNING;
    }
  }
};


DRAM_ATTR Node* tree = new Selector({ new Sequence({ new IsFoodAvailable(), new EatFood() }),
                                      new Sequence({ new IsHungry(), new AskForFood() }),
                                      new Sequence({ new BeingCarried(), new ComplainAboutBeingCarried() }),
                                      new Idle() });

void loop() {
  mpu.gyroUpdate();
  
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

  int gyroX = round(mpu.gyroX() + gyroXOffset) * -3;  //multiply gyro values by 2 for easier use, -2 to invert the angle
  int gyroY = round(mpu.gyroY() + gyroYOffset) * 4;
  int gyroZ = round(mpu.gyroZ() + gyroZOffset) * 3;

  unsigned long updateNow = millis();
  float deltaTime = (updateNow - lastUpdate) / 1000.0;
  lastUpdate = updateNow;

  angleX += gyroX * deltaTime;
  angleY += gyroY * deltaTime;
  angleZ += gyroZ * deltaTime;

  rgb.setPixelColor(0, rgb.Color(angleX, angleY, angleZ));  // set led color to angle of device
  rgb.show();

  updateButtonStates();

  display.clearDisplay();

  if (middleButtonState) {
    uiTimer = 100;
  }

  

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
    delay(1000);
  }

  delay(5);
}
