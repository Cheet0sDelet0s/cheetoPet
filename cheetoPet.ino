/**********************************************************
* cheetoPet - by olly jeffery / @Cheet0sDelet0s on github *
***********************************************************

an esp32 c3 based, feature-rich tamagotchi!

fully open source - do whatever you want with it!
you dont have to, but it would be great if you could credit me if you use any of this stuff!

repo: https://github.com/Cheet0sDelet0s/cheetoPet
website blog: https://www.cloudables.net/2025/08/22/cheetopet/ (why not have a look at the rest of the site as well!)

components:

esp32 c3 (use super mini dev board if not on pcb)
SH1107 128x128 oled display
DS3231 clock
AT24C32 EEPROM chip (included in lots of DS3231 modules if using one of them)
MPU9250 gyro
TP4056 charging board
1s lipo battery (1000mah will last a week or so without charging, go higher if you want)
3x smd push buttons
2 position 3 pin switch

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
#include "flappybird.h"
#include "particlesim.h"
#include "Picopixel.h"

#define SDA_ALT 20
#define SCL_ALT 9
#define LED_PIN 8
#define LED_COUNT 1
#define SWITCH_PIN GPIO_NUM_0
#define SPKR_PIN 3

Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define SCREEN_WIDTH 128   // display width
#define SCREEN_HEIGHT 128  // display height
#define OLED_RESET -1      // can set an oled reset pin if desired

#define EEPROM_ADDRESS 0x57 // Check your chip
#define EEPROM_SIZE 4096    // AT24C32 = 4KB

#define OFFSET 4   // screen recording offset, adjust until it lines up

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

bool spkrEnable = true;

struct Note {
  float freq;
  int length;
};

// Tone queue
#define MAX_TONES 48
Note noteQueue[MAX_TONES];
int toneCount = 0;  // number of tones currently in the queue

// Playback state
unsigned long lastStepTime = 0;
int currentTone = 0;
bool isPlaying = false;
DRAM_ATTR int playTime = 50;

Note odeToJoy[15] = {
  {329.63, 500}, {329.63, 500}, {349.23, 500}, {392.00, 500}, {392.00, 500}, {349.23, 500}, {329.63, 500},
  {293.66, 500}, {261.63, 500}, {261.63, 500}, {293.66, 500}, {329.63, 500}, {329.63, 750}, {293.66, 500},
  {293.66, 1000}
};

Note maryLamb[14] = { //written by AI
  {329.63, 500}, {293.66, 500}, {261.63, 500}, {293.66, 500}, {329.63, 500}, {329.63, 500}, {329.63, 1000},
  {293.66, 500}, {293.66, 500}, {293.66, 1000}, {329.63, 500}, {392.00, 500}, {392.00, 1000}, {329.63, 1000}
};

Note happyBirthday[25] = { //written by AI
  {264.00, 250}, {264.00, 250}, {297.00, 500}, {264.00, 500}, {352.00, 500}, {330.00, 1000}, 
  {264.00, 250}, {264.00, 250}, {297.00, 500}, {264.00, 500}, {396.00, 500}, {352.00, 1000},
  {264.00, 250}, {264.00, 250}, {528.00, 500}, {440.00, 500}, {352.00, 500}, {330.00, 500}, {297.00, 1000},
  {466.00, 250}, {466.00, 250}, {440.00, 500}, {352.00, 500}, {396.00, 500}, {352.00, 1000}
};

// Song metadata wrapper
struct Song {
  Note* notes;
  int length;
};

Song songs[] = {
  {odeToJoy, sizeof(odeToJoy) / sizeof(odeToJoy[0])},
  {maryLamb, sizeof(maryLamb) / sizeof(maryLamb[0])},
  {happyBirthday, sizeof(happyBirthday) / sizeof(happyBirthday[0])}
};

int numSongs = sizeof(songs) / sizeof(songs[0]);

DRAM_ATTR unsigned long previousMillis = 0;
const long interval = 50;  

DRAM_ATTR DateTime now;

int saveFileVersion = 1;

DRAM_ATTR unsigned long lastDump = 0;  // keeps track of the last time we dumped hahahah get it
bool screenRecording = false;

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

RTC_DS3231 rtc;
DateTime tempDateTime;
MPU9250_asukiaaa mpu(0x69);

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
DRAM_ATTR int lastSecond = -1;
DRAM_ATTR DateTime lastRunTime;
DRAM_ATTR int saveInterval = 5;

DRAM_ATTR int liveDataTimer = 0;

struct ItemList {
  int type, x, y;
  bool active;
};

DRAM_ATTR ItemList outsidePlot[30] = {
  {34, 40, 50, true},
  {35, 60, 70, true},
  {33, 96, 5, true},
  {36, 46, 6, true}
};

DRAM_ATTR int outsidePlotPlaced = 4;

//item inventory
DRAM_ATTR int inventory[8] = {};
DRAM_ATTR int inventoryItems = 0;
DRAM_ATTR ItemList placedHomeItems[30] = {
  {7, 98, 4, true}
};
DRAM_ATTR int amountItemsPlaced = 1;
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

DRAM_ATTR int* areaItemsPlaced;
DRAM_ATTR ItemList* currentAreaPtr = nullptr;

//game library
DRAM_ATTR int gameLibrary[8] = { 0, 1, 2, 3};
DRAM_ATTR int gameLibraryCount = 4;

const String gameNames[5] = {"pong", "veridium", "flappy bird", "bubblebox"};

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
DRAM_ATTR float loadIndicator = 0;

DRAM_ATTR int uiTimer = 100;

DRAM_ATTR int petHunger = 60;
DRAM_ATTR int petFun = 60;
DRAM_ATTR int petSleep = 60;
DRAM_ATTR int petX = 64;
DRAM_ATTR int petY = 32;
DRAM_ATTR bool showPetMenu = false;
DRAM_ATTR bool movingPet = false;
DRAM_ATTR float money = 40;
DRAM_ATTR int currentArea = 0;

/*AREAS:
0: inside home
1: outside plot

DOOR LOCATIONS:

0: top of screen
1: right of screen
2: bottom of screen
3: left of screen
*/

const int exitLocations[2] = {1, 0};
const int exitLinks[2] = {1, 0};

int doorX = 0;
int doorY = 0;
int doorW = 0;
int doorH = 0;

DRAM_ATTR int petMoveX = 64;
DRAM_ATTR int petMoveY = 32;
DRAM_ATTR int petMoveSpeed = 0;
DRAM_ATTR bool movePet = false;
DRAM_ATTR int petMoveAnim = 0;
DRAM_ATTR int petDir = 1;
DRAM_ATTR int petSitTimer = 0;
DRAM_ATTR int petSitType = 0;
DRAM_ATTR int petStatus = 0;

DRAM_ATTR unsigned long lastUpdate = 0;

DRAM_ATTR String currentPetMessage;
DRAM_ATTR int messageDisplayTime = 0;
DRAM_ATTR int messageMaxTime = 0;

DRAM_ATTR int constructionShopItems[] = { 3, 4, 5, 6, 7, 19, 30, 31, 34, 35, 37 };
DRAM_ATTR float constructionShopPrices[] = { 5, 2.50, 7.50, 10, 12.50, 4, 15, 7.50, 1, 1, 15 };
DRAM_ATTR int constructionShopLength = sizeof(constructionShopItems) / sizeof(constructionShopItems[0]);

DRAM_ATTR int foodShopItems[] = { 16, 18 };
DRAM_ATTR float foodShopPrices[] = { 1.50, 1.50};
DRAM_ATTR int foodShopLength = sizeof(foodShopItems) / sizeof(foodShopItems[0]);

//settings
DRAM_ATTR uint8_t gyroSensitivityX = 2;
DRAM_ATTR uint8_t gyroSensitivityY = 3;
DRAM_ATTR uint8_t gyroSensitivityZ = 3;
DRAM_ATTR uint8_t loopDelay = 5;

DRAM_ATTR SaveGame currentSaveGame;

//gamer rgb
const uint8_t fadeSteps = 100;        // Number of steps between colors
const uint16_t fadeInterval = 10;     // Time between steps (ms)
DRAM_ATTR unsigned long lastRGBUpdate = 0;

const uint32_t colors[] = {
  rgb.Color(255, 0, 0),     // Red
  rgb.Color(0, 255, 0),     // Green
  rgb.Color(0, 0, 255),     // Blue
  rgb.Color(255, 0, 255),   // Magenta
  rgb.Color(255, 255, 0),   // Yellow
  rgb.Color(0, 255, 255)    // Cyan
};
const uint8_t numColors = sizeof(colors) / sizeof(colors[0]);

// ----- Fade State -----
DRAM_ATTR uint8_t currentColor = 0;
DRAM_ATTR uint8_t step = 0;

struct UIButton {
  int x, y;          // Bitmap draw position
  int w, h;          // Hitbox width + height
  int bitmapId;      // Index into your bitmap array
  void (*onPress)(); // Action when pressed
};


// -------------------- MENU SYSTEM --------------------

/// ============================
/// Actions
/// ============================

// depth 0
void selectEdit()     { firstOption = 1; depth = 1; }
void selectMenu()     { firstOption = 2; depth = 1; }
void selectSettings() { firstOption = 3; depth = 1; }

// back buttons
void goBackToDepth0() { firstOption = 0; depth = 0; }
void goBackToEdit()   { firstOption = 1; depth = 1; }
void goBackToMenu()   { firstOption = 2; depth = 1; }

// depth 1: edit
void openInventory()  { secondOption = 1; depth = 2; }
void openShop()       { secondOption = 2; thirdOption = 1; depth = 2; }

// depth 1: menu
void openApple()      { secondOption = 1; depth = 2; }
void openController() { secondOption = 2; depth = 2; }

// depth 2: inventory/shop
void reopenInventory(){ firstOption = 1; depth = 1; }
void reopenShop()     { firstOption = 1; depth = 1; }

// depth 2: food menu
void reopenFood()     { firstOption = 2; depth = 1; }
void reopenGames()    { firstOption = 2; depth = 1; }

/// ============================
/// Button definitions
/// ============================

// Depth 0 (root menu)
UIButton depth0Buttons[] = {
  {0,   115, 41, 15,  9,  selectEdit},     // Pencil
  {41,  115, 45, 15,  8,  selectMenu},     // Menu
  {86,  115, 41, 15, 10,  selectSettings}, // Settings
};

// Depth 1 → Edit submenu
UIButton editMenuButtons[] = {
  {0,   115, 41, 15, 12, goBackToDepth0}, // Back
  {41,  115, 45, 15, 13, openInventory},  // Inventory
  {86,  115, 41, 15, 11, openShop},       // Shop
};

// Depth 1 → Main menu submenu
UIButton mainMenuButtons[] = {
  {0,   115, 41, 15, 12, goBackToDepth0}, // Back
  {41,  115, 45, 15, 16, openApple},      // Apple
  {86,  115, 41, 15, 20, openController}, // Controller
};

// Depth 1 → Settings submenu
UIButton settingsButtons[] = {
  {0,   115, 41, 15, 12, goBackToDepth0}, // Back
};

// Depth 2 → Inventory
UIButton inventoryButtons[] = {
  {0,   115, 41, 15, 12, reopenInventory}, // Back
};

// Depth 2 → Shop
UIButton shopButtons[] = {
  {0,   115, 41, 15, 12, reopenShop}, // Back
};

// Depth 2 → Food inventory
UIButton foodButtons[] = {
  {0,   115, 41, 15, 12, reopenFood}, // Back
};

// Depth 2 → Game library
UIButton gameButtons[] = {
  {0,   115, 41, 15, 12, reopenGames}, // Back
};

// Helper to extract R/G/B from uint32_t
uint8_t getR(uint32_t color) { return (color >> 16) & 0xFF; }
uint8_t getG(uint32_t color) { return (color >> 8) & 0xFF; }
uint8_t getB(uint32_t color) { return color & 0xFF; }


void clearTones() {
  toneCount = 0;
  ledcWrite(SPKR_PIN, 0);
}

void queueTone(float freq, int length) {
  if (toneCount < MAX_TONES) {
    noteQueue[toneCount] = { freq, length };
    toneCount++;
  }
}

void priorityQueueTone(float freq, int length) {
  if (toneCount < MAX_TONES) {
    // Shift elements to the right
    for (int i = MAX_TONES - 1; i > 0; i--) {
      noteQueue[i] = noteQueue[i - 1];
    }

    Note newNote = {freq, length};

    // Insert at the start
    noteQueue[0] = newNote;
    toneCount++;
  }
}

void audioEngine() {
  if (toneCount > 0 && spkrEnable) {
    if (!isPlaying) {
      // Start playing first note
      currentTone = noteQueue[0].freq;
      playTime = noteQueue[0].length;
      ledcWriteTone(SPKR_PIN, currentTone);

      //Serial.print("Tone: "); Serial.print(currentTone);
      //Serial.print(" Length: "); Serial.println(playTime);

      lastStepTime = millis();
      isPlaying = true;
    } else if (millis() - lastStepTime >= playTime) {
      // Shift queue down by 1
      for (int i = 1; i < toneCount; i++) {
        noteQueue[i - 1] = noteQueue[i];
      }
      toneCount--;

      if (toneCount > 0) {
        // Play next note
        currentTone = noteQueue[0].freq;
        playTime = noteQueue[0].length;
        ledcWriteTone(SPKR_PIN, currentTone);

        //Serial.print("Tone: "); Serial.print(currentTone);
        //Serial.print(" Length: "); Serial.println(playTime);

        lastStepTime = millis();
      } else {
        // Finished all notes
        ledcWriteTone(SPKR_PIN, 0);
        isPlaying = false;
      }
    }
  } else if (!spkrEnable) {
    ledcWrite(SPKR_PIN, 0);
  }
}

void playRandomSong() {
  clearTones();

  int songIndex = random(numSongs); // Pick random song
  Song chosen = songs[songIndex];

  for (int i = 0; i < chosen.length; i++) {
    queueTone(chosen.notes[i].freq, chosen.notes[i].length);
  }
}

void peripheralTest() {
  waitForSelectRelease();
  while (!rightButtonState) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(SH110X_WHITE, SH110X_BLACK);
    display.setTextSize(2);
    updateGyro();
    
    now = rtc.now();
    display.print(now.hour());
    display.print(":");
    if (now.minute() < 10) {
      display.print("0");
      display.println(now.minute());
    } else {
      display.println(now.minute());
    }
    display.print("X: "); display.println(angleX);
    display.print("Y: "); display.println(angleY);
    display.print("Z: "); display.println(angleZ);
    
    display.display();

    screenRecord();

    updateButtonStates();
    
    delay(5);
  }
}

void printItemList(const ItemList* list, int length) {
  Serial.println("Item List:");
  for (int i = 0; i < length; i++) {
    Serial.print("Item ");
    Serial.print(i);
    Serial.print(" -> ");
    Serial.print("Type: ");
    Serial.print(list[i].type);
    Serial.print(", X: ");
    Serial.print(list[i].x);
    Serial.print(", Y: ");
    Serial.print(list[i].y);
    Serial.print(", Active: ");
    Serial.println(list[i].active ? "true" : "false");
  }
}

void updateAreaPointers() {
  if (currentArea == 0) {
    areaItemsPlaced = &amountItemsPlaced;
    currentAreaPtr = placedHomeItems;
  } else if (currentArea == 1) {
    areaItemsPlaced = &outsidePlotPlaced;
    currentAreaPtr = outsidePlot;
  }
}

void updateDoorDimensions(int location) {
  if (location == 0) {
    doorX = 59;
    doorH = 15;
    doorY = 42 - doorH;
    doorW = 10;
  } else if (location == 1) {
    doorX = 124;
    doorH = 10;
    doorY = 59;
    doorW = 3;
  } else if (location == 2) {
    doorX = 59;
    doorH = 2;
    doorY = 63;
    doorW = 10;
  } else if (location == 3) {
    doorX = 0;
    doorH = 10;
    doorY = 59;
    doorW = 3;
  }
}

void testdrawline() {   // -- by adafruit from their SH1107_128x128.ino example
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SH110X_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, SH110X_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, SH110X_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, SH110X_WHITE);
    display.display();
    delay(1);
  }

  delay(2000); // Pause for 2 seconds
}

void petMessage(String message) {
  currentPetMessage = message;
  messageDisplayTime = 0;
  int msgLength = currentPetMessage.length();
  messageMaxTime = constrain((msgLength * 15) / 2, 1, 3000);

  for (int i = 0; i < msgLength; i++) {
    int letterNumber = currentPetMessage[i] - 'a';
    int frequency = 400 + letterNumber * 4;
    queueTone(frequency, 50);
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

void blindCloseAnimation() {
  const int bars = 32;                      // Number of horizontal bars (blinds)
  const int barHeight = 128 / bars;         // Height of each bar
  
  display.clearDisplay();
  display.display();

  for (int i = 0; i < bars; i++) {
    // Draw filled rectangle for the "blind" closing effect
    display.fillRect(0, i * barHeight, 128, barHeight, SH110X_WHITE);
    display.display();
    delay(10);  // Speed of closing - adjust for faster/slower animation
  }
}

void blindOpenAnimation() {
  const int bars = 32;
  const int barHeight = 128 / bars;

  // Start with all blinds closed
  display.clearDisplay();
  for (int i = 0; i < bars; i++) {
    display.fillRect(0, i * barHeight, 128, barHeight, SH110X_WHITE);
  }
  display.display();

  // Open blinds from bottom to top (reverse of shutting)
  for (int i = bars - 1; i >= 0; i--) {
    display.fillRect(0, i * barHeight, 128, barHeight, SH110X_BLACK);
    display.display();
    delay(10);
  }
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

void sitPet(int time, int type = 26) {
  petSitTimer = time;
  petSitType = type;
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

  for (int i = 0; i < amountItemsPlaced; i++) {
    currentSaveGame.placed[i] = placedHomeItems[i].type;
    currentSaveGame.placedX[i] = placedHomeItems[i].x;
    currentSaveGame.placedY[i] = placedHomeItems[i].y;
  }

  currentSaveGame.placedItems = amountItemsPlaced;

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
  
  for (int i = 0; i < currentSaveGame.placedItems; i++) {
    placedHomeItems[i] = {
      (int)currentSaveGame.placed[i],
      (int)currentSaveGame.placedX[i],
      (int)currentSaveGame.placedY[i],
      true
    };
  }

  amountItemsPlaced = currentSaveGame.placedItems;

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

void prepareForSleepyTime() {
  display.clearDisplay();
  display.display();
  rgb.setPixelColor(0, rgb.Color(0, 0, 0));
  rgb.show();
  mpu9250_sleep();
}

bool addToList(int list[], int& itemCount, int maxSize, int value) {
  if (itemCount < maxSize) {
    list[itemCount++] = value;
    return true;
  } else {
    return false;
  }
}

int indexOf(ItemList array[], int length, int targetType) {
  for (int i = 0; i < length; i++) {
    if (array[i].active && array[i].type == targetType) {
      return i;
    }
  }
  return -1; // Not found
}

int indexOfList(int array[], int length, int target) {
  for (int i = 0; i < length; i++) {
    if (array[i] == target) {
      return i;
    }
  }
  return -1;  // Not found
}


void setup() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(middleButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(SPKR_PIN, OUTPUT);

  ledcAttach(SPKR_PIN, 5000, 8);

  ledcWriteTone(SPKR_PIN, 0);           // start silent

  Serial.begin(921600);
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
  dumpBufferASCII();
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
    display.println("rtc module lost power! time, date and save data has been reset. oh dear. booting in 5 secs");
    display.println("make sure the coin cell didnt fall out or has lost charge!");
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
  bool readyToStart = false;
  while (!readyToStart) {
      updateButtonStates();
      if (leftButtonState) {
      //load game
      loadGameFromEEPROM();
      break;
    } else if (rightButtonState) {
      display.clearDisplay();
      drawCenteredText(display, "are you sure?", 0);
      drawCenteredText(display, "A = yes", 10);
      drawCenteredText(display, "B = no", 20);
      display.display();
      dumpBufferASCII();
      bool confirmed = false;
      waitForSelectRelease();
      while (!confirmed) {
        updateButtonStates();
        if (rightButtonState) {confirmed = true;}
        if (leftButtonState) {break;}
        delay(50);
      }
      if (confirmed) {
        saveGameToEEPROM();
        readyToStart = true;
        break;
      }      
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
      drawCenteredText(display, "X: RAM game", 105);
      display.display();
      screenRecord();
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

bool drawButton(int x, int y, int w, int h, const char* label) {
  display.setCursor(x, y);
  bool touched = detectCursorTouch(x, y, w, h);
  if (touched) {
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(label);
    updateButtonStates();
    if (rightButtonState) {
      waitForSelectRelease();
      return true;
    }
    return false;
  } else {
    display.setTextColor(SH110X_WHITE);
    display.print(label);
    return false;
  }
}

void drawBitmapButton(const UIButton& btn) {
  const BitmapInfo& bmp = bitmaps[btn.bitmapId];

  // Draw bitmap
  display.drawBitmap(btn.x + (btn.w / 2 - bmp.width / 2), btn.y, bmp.data, bmp.width, bmp.height, SH110X_WHITE);

  // Check hitbox
  if (detectCursorTouch(btn.x, btn.y - 2, btn.w, btn.h)) {
    if (rightButtonState) {
      btn.onPress();
      waitForSelectRelease();
    } else {
      // highlight at hitbox edges
      display.drawFastVLine(btn.x + 2, 115, 11, SH110X_WHITE);
      display.drawFastVLine(btn.x + btn.w - 2, 115, 11, SH110X_WHITE);
    }
  }
}

void drawButtonSet(UIButton* buttons, int count) {
  drawBottomBar();
  for (int i = 0; i < count; i++) {
    drawBitmapButton(buttons[i]);
  }
}

// Adjustable control with + / value / -
bool drawAdjustable(int x, int y, int& value, int minVal, int maxVal, const char* label, bool selected = false) {
  // Label (optional, can be empty string)
  if (label && strlen(label) > 0) {
    
    display.setCursor(x - (String(label).length() * 6) - 1, y);
    display.setTextColor(SH110X_WHITE);
    display.print(label);
  }

  // + button
  if (drawButton(x, y - 10, 20, 10, "+")) {
    value++;
    if (value > maxVal) value = minVal;  // wrap around
    return true;
  }

  // Value
  display.setCursor(x, y);
  if (selected) {
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
  } else {
    display.setTextColor(SH110X_WHITE);
  }
  display.print(value);

  // - button
  if (drawButton(x, y + 10, 20, 10, "-")) {
    value--;
    if (value < minVal) value = maxVal;  // wrap around
    return true;
  }

  return false;
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
    display.println("press A to\nrestart.");
    display.display();
    updateButtonStates();
    while(!rightButtonState) {updateButtonStates();}
    esp_restart();
}

bool detectCursorTouch(int startX, int startY, int endX, int endY) {
  if (cursorX > startX && cursorX < startX + endX && cursorY > startY && cursorY < startY + endY) {
    return true;
  } else {
    return false;
  }
}

void drawCursor() {
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
  display.fillRect(cursorX, cursorY + bmp.height + 3, loadIndicator, 2, SH110X_WHITE);
}

void drawPet(int petNumber, int drawX, int drawY) {
  switch (petNumber) {
    case 0:
      {
        if (!movePet) {
          if (petSitTimer > 0) {
            drawBitmapFromList(drawX - 1, drawY - 1, petDir, petSitType + 1, SH110X_BLACK);
            drawBitmapFromList(drawX, drawY, petDir, petSitType, SH110X_WHITE);
            petSitTimer--;
          } else {
            drawBitmapWithDirection(drawX - 1, drawY - 1, petDir, pet_gooseStillBigMask, 18, 28, SH110X_BLACK);
            drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseStillBig, 16, 26, SH110X_WHITE);
          }
        } else {
          if (petMoveAnim < 3) {
            drawBitmapWithDirection(drawX - 1, drawY - 1, petDir, pet_gooseWalkMask, 18, 27, SH110X_BLACK);
            drawBitmapWithDirection(drawX, drawY, petDir, pet_gooseWalk, 16, 26, SH110X_WHITE);
          } else {
            drawBitmapWithDirection(drawX - 1, drawY, petDir, pet_gooseWalk2Mask, 19, 26, SH110X_BLACK);
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
      drawButtonSet(depth0Buttons, 3);
      break;

    case 1:
      switch (firstOption) {
        case 1: // edit submenu
          drawButtonSet(editMenuButtons, 3);
          break;
        case 2: // main menu submenu
          drawButtonSet(mainMenuButtons, 3);
          break;
        case 3: // settings
          updateButtonStates();
          if (!startHandlingPlacing) drawSettings();
          drawButtonSet(settingsButtons, 1);
          break;
      }
      break;

    case 2:
      switch (firstOption) {
        case 1: // inventory/shop branch
          switch (secondOption) {
            case 1: // inventory
              updateButtonStates();
              if (!startHandlingPlacing) drawInventory();
              drawButtonSet(inventoryButtons, 1);
              break;
            case 2: // shop
              updateButtonStates();
              if (!startHandlingPlacing) drawShop();
              drawButtonSet(shopButtons, 1);
              break;
          }
          break;

        case 2: // food/games branch
          switch (secondOption) {
            case 1: // food inventory
              updateButtonStates();
              if (!handleFoodPlacing) drawFoodInventory();
              drawButtonSet(foodButtons, 1);
              break;
            case 2: // game library
              updateButtonStates();
              if (!handleFoodPlacing) drawGameLibrary();
              drawButtonSet(gameButtons, 1);
              break;
          }
          break;
      }
      break;
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
  display.setCursor(0, -2);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  display.setFont(&Picopixel);
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

void drawAreaItems() {
  display.drawFastHLine(0, 42, 127, SH110X_WHITE);

  updateAreaPointers();
  updateButtonStates();


  for (int i = 0; i < *areaItemsPlaced; i++) {
    uint8_t type = currentAreaPtr[i].type;
    if (type == 0) continue;  // stop foolish behavoir1!1!!!1!

    //if (type >= NUM_BITMAPS) continue;  // avoid out-of-bounds bitmap access

    const BitmapInfo& bmp = bitmaps[type];
    int x = currentAreaPtr[i].x;
    int y = currentAreaPtr[i].y;

    display.drawBitmap(x, y, bmp.data, bmp.width, bmp.height, SH110X_WHITE);

    if (type == 5) {
      display.fillRect(x + 6, y, 3, -27, SH110X_WHITE);
    }

    if (detectCursorTouch(x, y, bmp.width, bmp.height)) {
      if (rightButtonState && loadIndicator > 9) {
        // remove item
        loadIndicator = 0;
      } else if (rightButtonState) {
        loadIndicator++;
      }
    }
  }

  updateDoorDimensions(exitLocations[currentArea]);

  if (exitLocations[currentArea] != 0) {
    display.fillRect(doorX, doorY, doorW, doorH, SH110X_WHITE);
  }

  if (detectCursorTouch(doorX, doorY, doorW, doorH)) {
    if (rightButtonState && loadIndicator > 9) {
      currentArea = exitLinks[currentArea];
      loadIndicator = 0;
      blindCloseAnimation();
      updateDoorDimensions(exitLocations[currentArea]);
      petX = doorX;
      petY = doorY;
    } else if (rightButtonState) {
      loadIndicator++;
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

  startHandlingPlacing = false;

  updateAreaPointers();

  ItemList newItem = {
    itemBeingPlaced,
    cursorX,
    (itemBeingPlaced == 5 ? 27 : cursorY),
    true
  };

  currentAreaPtr[*areaItemsPlaced] = newItem;

  (*areaItemsPlaced)++;

  removeFromList(inventory, inventoryItems, indexOfList(inventory, inventoryItems, itemBeingPlaced));
  inventoryItems--;
  itemBeingPlaced = -1;
  waitForSelectRelease();
}

void handleFoodPlacingLogic() {
  Serial.println(rightButtonState);
  Serial.println(itemBeingPlaced);

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
  removeFromList(foodInventory, foodInventoryItems, indexOfList(foodInventory, foodInventoryItems, itemBeingPlaced));
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
  display.drawFastHLine(petMenuX + 1, petMenuY + 20, 28, SH110X_BLACK);
  display.setTextColor(SH110X_BLACK);
  display.setTextSize(1);
  display.setCursor(petMenuX + 1, petMenuY + 1);
  display.print("move");
  display.setCursor(petMenuX + 1, petMenuY + 11);
  display.print("pet");
  display.setTextColor(SH110X_WHITE);
  updateButtonStates();
  if (rightButtonState) {
    if (!detectCursorTouch(petMenuX, petMenuY, 30, 50)) {
      showPetMenu = false;
    } else if (detectCursorTouch(petMenuX, petMenuY, 30, 10)) {
      movingPet = true;
      waitForSelectRelease();
      showPetMenu = false;
    } else if (detectCursorTouch(petMenuX, petMenuY + 10, 30, 10)) {
      petFun += random(0, 3);
      showPetMenu = false;
      petMessage("glorb");
      waitForSelectRelease();
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
  display.setFont(&Picopixel);
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
  display.setFont(NULL);
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
  display.print("/3");

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
      } else if (name == "veridium") {
        drawCheckerboard(petVeridiumLVL + 1);
        veridium();
      } else if (name == "flappy bird") {
        drawCheckerboard(3);
        flappyBird();
      } else if (name == "bubblebox") {
        drawCheckerboard(3);
        particleSim();
      } else {
        //latinProject();
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

  if (detectCursorTouch(1, 10, 54, 8) && rightButtonState) {
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
    display.print("construct");
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
    display.print("construct ");
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

  return buttonClicked;
}

//SETTINGS PAGES

void settingsMainMenu() {
  int y = 10;

  if (drawButton(0, y, 102, 8, "set date and time")) settingsOption = 1;
  y += 8;

  if (drawButton(0, y, 102, 8, "gyro sensitivity")) settingsOption = 2;
  y += 8;

  if (drawButton(0, y, 102, 8, "performance")) settingsOption = 3;
  y += 8;

  if (drawButton(0, y, 102, 8, "save manager")) settingsOption = 4;
  y += 8;

  if (drawButton(0, y, 102, 8, "display")) settingsOption = 5;
  y += 8;

  if (drawButton(0, y, 102, 8, "misc")) settingsOption = 6;
  y = 84;

  // Toggle screen recording
  if (drawButton(0, y, 102, 8, screenRecording ? "stop record" : "start record")) {
    screenRecording = !screenRecording;
  }
  y += 8;

  // Toggle speaker
  if (drawButton(0, y, 102, 8, spkrEnable ? "mute" : "unmute")) {
    spkrEnable = !spkrEnable;
  }
  y += 8;

  // Restart
  if (drawButton(0, y, 102, 8, "restart")) {
    esp_restart();
  }
}

void settingsDateTime() {
  static bool dateTimeInitialized = false;
  static int year, month, day, hour, minute;

  if (!dateTimeInitialized) {
    DateTime now = rtc.now();
    year = now.year();
    month = now.month();
    day = now.day();
    hour = now.hour();
    minute = now.minute();
    dateTimeInitialized = true;
  }

  display.setCursor(0, 10);
  display.print("edit date & time");
  //   X VALUES   Y  Mon  D  H   Min
  int fieldX[] = {0, 33, 55, 85, 100};
  int fieldY   = 40;

  // Year
  drawAdjustable(fieldX[0], fieldY, year, 2000, 2099, "", selectedField == 0);

  // Month
  drawAdjustable(fieldX[1], fieldY, month, 1, 12, "", selectedField == 1);

  // Day
  drawAdjustable(fieldX[2], fieldY, day, 1, 31, "", selectedField == 2);

  // Hour
  drawAdjustable(fieldX[3], fieldY, hour, 0, 23, "", selectedField == 3);

  // Minute
  drawAdjustable(fieldX[4], fieldY, minute, 0, 59, "", selectedField == 4);

  // Confirm button
  if (drawButton(30, 80, 60, 10, "confirm")) {
    rtc.adjust(DateTime(year, month, day, hour, minute, 0));
    settingsOption = 0;
    selectedField = 0;
    dateTimeInitialized = false;
  }
}

void settingsGyro() {  // LOOK. AT. THAT. BEAUTIFUL
  static int sensitivityX = gyroSensitivityX;
  static int sensitivityY = gyroSensitivityY;
  static int sensitivityZ = gyroSensitivityZ;
  
  display.setCursor(0, 10);
  display.print("gyro sensitivity");
  
  drawAdjustable(20, 50, sensitivityX, -5, 5, "X:", false);
  drawAdjustable(50, 50, sensitivityY, -5, 5, "Y:", false);
  drawAdjustable(80, 50, sensitivityZ, -5, 5, "Z:", false);

  if (drawButton(30, 80, 60, 10, "confirm")) {
      settingsOption = 0;
      waitForSelectRelease();
  }

  updateButtonStates();
}

void settingsPerformance() { //pretty good wheyyy
  static int delayTemp = loopDelay;     

  display.setCursor(0, 10);
  display.print("loop delay");

  drawAdjustable(80, 50, delayTemp, 0, 100, "delay (ms):", false);

  if (drawButton(30, 80, 60, 10, "confirm")) {
    loopDelay = delayTemp;
    settingsOption = 0;
    waitForSelectRelease();
  }

  updateButtonStates();
}

void settingsSaveManager() {    //OH MY GOD THIS IS SO CLEAN AGHHHHH
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

  drawAdjustable(70, 75, saveInterval, 1, 60, "interval:", false);

  if (drawCenteredButton("exit", 100)) {
    settingsOption = 0;
  }

  updateButtonStates();
}

void settingsDisplay() {
  static int brightnessTemp = 5;

  display.setCursor(0, 10);
  display.print("display");

  drawAdjustable(80, 30, brightnessTemp, 1, 10, "brightness:", false);

  display.setContrast(brightnessTemp * 25.5);

  if (drawCenteredButton("exit", 90)) {
    settingsOption = 0;
  }

  updateButtonStates();
}

void settingsMisc() {
  display.setCursor(0, 10);
  display.println("misc\n");

  updateButtonStates();
  
  if (drawCenteredButton("deep sleep", 20)) {
    display.clearDisplay();
    display.print("the device will enter deep sleep in 5 seconds. to turn it back on, disconnect and reconnect the battery.");
    display.display();
    delay(5000);
    prepareForSleepyTime();
    Serial.println("going into deep sleep as requested by user goodnight");
    esp_deep_sleep_start();
  }

  if (drawCenteredButton("display test", 32)) {
    display.clearDisplay();
    display.display();
    testdrawline();
  }
  
  if (drawCenteredButton("peripheral test", 44)) {
    peripheralTest();
  }

  if (drawCenteredButton("random song", 56)) {
    playRandomSong();
  }

  if (drawCenteredButton("exit", 90)) {
    settingsOption = 0;
  }
}

void drawSettings() {
  uiTimer = 100;
  display.fillRect(0, 0, 128, 112, SH110X_BLACK);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("settings");
  display.drawFastHLine(0, 8, 127, SH110X_WHITE);
  display.setCursor(0, 10);
  updateButtonStates();

  switch (settingsOption) {
    case 0: settingsMainMenu(); break;
    case 1: settingsDateTime(); break;
    case 2: settingsGyro(); break;
    case 3: settingsPerformance(); break;
    case 4: settingsSaveManager(); break;
    case 5: settingsDisplay(); break;
    case 6: settingsMisc(); break;
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

bool checkItemIsPlaced(int item) {
  updateAreaPointers();
  for (int i = 0; i < *areaItemsPlaced; i++) {
    if (currentAreaPtr[i].active && currentAreaPtr[i].type == item) {
      return true;
    }
  }
  return false;
}

void dumpBufferASCII() {
  uint8_t *buffer = display.getBuffer();
  const int w = display.width();
  const int h = display.height();
  const int rowsPerChunk = 8;  // how many rows to print at once
  char chunk[w * rowsPerChunk + rowsPerChunk + 1]; // +1 for final null, +rowsPerChunk for line breaks

  for (int y = 0; y < h; y += rowsPerChunk) {
    int chunkIndex = 0;

    for (int row = 0; row < rowsPerChunk && (y + row) < h; row++) {
      for (int x = 0; x < w; x++) {
        int byteIndex = x + ((y + row) / 8) * w;
        int bitMask = 1 << ((y + row) & 7);
        chunk[chunkIndex++] = (buffer[byteIndex] & bitMask) ? '#' : '.';
      }
      chunk[chunkIndex++] = '\n'; // add newline at end of row
    }

    chunk[chunkIndex] = '\0';  // null-terminate the chunk
    Serial.print(chunk);        // print multiple rows at once
  }

  Serial.println(); // final empty line to signal end of frame
}

void screenRecord() {
  if (millis() - lastDump >= 100 && screenRecording) {
      lastDump = millis();
      dumpBufferASCII();
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
    return (petHunger < 30) ? SUCCESS : FAILURE;
  }
};

class IsBored : public Node {
public:
  NodeStatus tick() override {
    return (petFun < 40) ? SUCCESS : FAILURE;
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
    if (messageDisplayTime < messageMaxTime || random(0, 50) != 1) {
      return RUNNING;
    }
    int messageRandomiser = random(0, hungryLinesCount);
    petMessage(hungryLines[messageRandomiser]);
    return SUCCESS;   
  } 
};

class AskForPlay : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime < messageMaxTime || random(0, 50) != 1) {
      return RUNNING;
    }
    int messageRandomiser = random(0, boredLinesCount);
    petMessage(boredLines[messageRandomiser]);
    return SUCCESS;
  } 
};

class AskForSleep : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime < messageMaxTime || random(0, 50) != 1) {
      return RUNNING;
    }
    int messageRandomiser = random(0, tiredLinesCount);
    petMessage(tiredLines[messageRandomiser]);
    return SUCCESS;
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
    if (!movePet) {
      if (random(0, 100) == 1) {
        startMovingPet(random(0, 105), random(35, 100), 1);
      }
    }
    return SUCCESS;
  }
};

class WantToUsePiano : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 3 || (petStatus == 0 && (petSitTimer < 5) && random(0, 800) == 1)) {
      petStatus = 3;
      return SUCCESS;
    }
    return FAILURE;
  }
};

class UsePiano : public Node {
public:
  NodeStatus tick() override {
    if (checkItemIsPlaced(37)) {
      updateAreaPointers();
      int index = indexOf(currentAreaPtr, *areaItemsPlaced, 37);
      int itemX = currentAreaPtr[index].x + 4;
      int itemY = currentAreaPtr[index].y + 10;
      if (!movePet) {
        startMovingPet(itemX, itemY, 1);
      }
      //Serial.println("moving pet to piano");
      if (petX == itemX && petY == itemY) {
        //Serial.println("pet has reached piano");
        
        playRandomSong();

        petMessage(pianoLines[random(0, pianoLinesCount)]);
        petStatus = 0;
        return SUCCESS;
      } 
      return RUNNING;
    }
    petStatus = 0;
    return FAILURE;
  }
};

class WantToUseFireplace : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 2 || (petStatus == 0 && (petSitTimer < 5) && random(0, 800) == 1)) {
      petStatus = 2;
      return SUCCESS;
    }
    return FAILURE;
  }
};

class UseFireplace : public Node {
public:
  NodeStatus tick() override {
    if (checkItemIsPlaced(5)) {
      updateAreaPointers();
      int index = indexOf(currentAreaPtr, *areaItemsPlaced, 5);
      int itemX = currentAreaPtr[index].x - 15;
      int itemY = currentAreaPtr[index].y + 3;
      if (!movePet) {
        startMovingPet(itemX, itemY, 2);
      }
      //Serial.println("moving pet to fireplace");
      if (petX == itemX && petY == itemY) {
        //Serial.println("pet has reached fireplace");
        sitPet(200, 28);
        petHunger += 1;
        petMessage(fireplaceLines[random(0, fireplaceLinesCount)]);
        petStatus = 0;
        return SUCCESS;
      } 
      return RUNNING;
    }
    petStatus = 0;
    return FAILURE;
  }
};

class WantToSitOnCouch : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 1 || (petStatus == 0 && (petSitTimer < 5) && random(0, 800) == 1)) {
      //Serial.println("wanting to sit on couch");
      petStatus = 1;
      return SUCCESS;
    }
    //Serial.println("not wanting to sit on couch");
    return FAILURE;
  }
};

class IsCouchAvailable : public Node {
public:
  NodeStatus tick() override {
    updateAreaPointers();
    if (indexOf(currentAreaPtr, *areaItemsPlaced, 3) != -1) {
      return SUCCESS; 
    } else {
      petStatus = 0;
      return FAILURE;
    }
  }
};

class SitOnCouch : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 1) {
      updateAreaPointers();
      int index = indexOf(currentAreaPtr, *areaItemsPlaced, 3);
      if (index != -1) {
        int itemX = currentAreaPtr[index].x + 4;
        int itemY = currentAreaPtr[index].y + 2;
        if (!movePet) {
          startMovingPet(itemX, itemY, 2);
        }
        //Serial.println("moving pet to couch");
        if (petX == itemX && petY == itemY) {
          //Serial.println("pet has reached couch");
          sitPet(200);
          petStatus = 0;
          return SUCCESS;
        } else {
          return RUNNING;
        }
      }  
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

/* THIS IS THE SHIT. RIGHT HERE. THIS IS WHERE ITS AT. OH MY GOD LOOK AT IT
*ahem* behaviour tree setup. put all actions in order, the higher they are in the list the higher their priority.
each node will pass to the next one FAILURE, RUNNING or SUCCESS.
if its failure, it runs the next sequence below it.
if its running, it doesnt run the next node or next sequence.
if its success, it runs the next node in the current sequence.
for example, dying is a higher priority than asking for food.
*/
DRAM_ATTR Node* tree = new Selector({ new Sequence({ new ShouldDie(), new Die() }),
                                      new Sequence({ new IsBeingShaken(), new ComplainAboutBeingShaken() }),
                                      new Sequence({ new BeingCarried(), new ComplainAboutBeingCarried() }),
                                      new Sequence({ new IsFoodAvailable(), new EatFood() }),
                                      new Sequence({ new IsHungry(), new AskForFood() }),
                                      new Sequence({ new IsTired(), new AskForSleep() }),
                                      new Sequence({ new IsBored(), new AskForPlay() }),
                                      new Sequence({ new WantToUseFireplace(), new UseFireplace() }) ,
                                      new Sequence({ new WantToSitOnCouch(), new IsCouchAvailable(), new SitOnCouch() }),
                                      new Sequence({ new WantToUsePiano(), new UsePiano() }),
                                      new Idle()
                                      });

void handleSleepMode() {
  if (powerSwitchState) {
    clearTones();
    blindCloseAnimation();
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
    dumpBufferASCII();
    delay(1000);
    prepareForSleepyTime();
    DateTime timeWhenSlept = rtc.now();
    gpio_wakeup_enable(SWITCH_PIN, GPIO_INTR_HIGH_LEVEL); // wake device up when switch goes high

    esp_sleep_enable_gpio_wakeup();

    esp_light_sleep_start();  // sleepy time! program will resume after wake up

    mpu9250_wake();
    DateTime now = rtc.now();

    TimeSpan timeSinceSlept = now - timeWhenSlept;

    int32_t seconds = timeSinceSlept.totalseconds();
    int32_t minutesSinceSlept = seconds / 60;

    petSleep += minutesSinceSlept;
    
    petHunger -= minutesSinceSlept / 30;
    
    petSleep = constrain(petSleep, 0, 120);

    blindOpenAnimation();

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
    dumpBufferASCII();

    if (minutesSinceSlept >= saveInterval) {
      delay(700);
      saveGameToEEPROM(true);
    } else {
      delay(1000);
    }
  }
}

void loop() {
  if (totalG > 11) {   //kinda funny but annoying. kills the pet if the device experiences over 11 g's of force. hard to do on accident unless you're in a fighter jet or something
    killPet("got shaken to death"); 
  }

  if (loadIndicator > 0) { // load indicator is the little bar below the cursor. ticks it down by 0.5 
    loadIndicator -= 0.5;
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

  updatePetNeeds(); // update pet fun hun slp ect

  if (currentMillis - lastRGBUpdate >= fadeInterval) { // handle RGB led on some dev boards
    lastRGBUpdate = currentMillis;

    // Get start and end colors
    uint32_t colorStart = colors[currentColor];
    uint32_t colorEnd = colors[(currentColor + 1) % numColors];

    // Linear interpolate between the colors
    float t = step / float(fadeSteps);
    uint8_t r = (1 - t) * getR(colorStart) + t * getR(colorEnd);
    uint8_t g = (1 - t) * getG(colorStart) + t * getG(colorEnd);
    uint8_t b = (1 - t) * getB(colorStart) + t * getB(colorEnd);

    rgb.setPixelColor(0, rgb.Color(r, g, b));
    rgb.show();

    step++;
    if (step > fadeSteps) {
      step = 0;
      currentColor = (currentColor + 1) % numColors;
    }
  }

  updateButtonStates(); // update button states

  display.clearDisplay(); // clear the display before calling all functions that draw to it. this happens every tick.

  if (middleButtonState) { // if you press X it shows the UI again by setting the UI timeout to 100.
    uiTimer = 100;
  }

  updateGyro(); // update gyro values, x y and z

  if (!leftButtonState) { // handle cursor movement. if B isnt held, set gyro values to 0.
    angleX = 0;
    angleY = 0;
    angleZ = 0;
    if (cursorTimer > 0) { // only draw cursor if cursor timeout is above 0.
      shouldDrawCursor = true;
      cursorTimer = cursorTimer - 0.1;
    } else {
      shouldDrawCursor = false;
    }
  } else {
    cursorX = constrain(angleX + 64, 0, 126); // make sure cursor doesn't go offscreen.
    if (itemBeingPlaced == 5) { // item 5 is the fireplace. you can only place the fireplace on the wall, so this constrains it.
      cursorY = 27;
    } else {
      cursorY = constrain(angleY + 64, 0, 126); // if you aren't placing a fireplace, allow free Y movement of the cursor.
    }
    shouldDrawCursor = true;
    cursorTimer = 4;  //how long cursor is displayed after releasing button, 1 = 50ms, 4 = 200ms, so on
  }

  drawAreaItems(); // draw the current area items
  
  if (movingPet) { // if the user is manually moving the pet, set the pets position to the cursors position. if the user presses A, stop moving the pet.
    petX = cursorX;
    petY = cursorY;
    constrain(petY, 43, 127);
    updateButtonStates();
    if (rightButtonState) {
      movingPet = false;
    }
  }

  drawPet(0, petX, petY); // draw the pet to the display in its current state

  if (messageDisplayTime < messageMaxTime) {
    drawPetMessage(); // draw speech bubble on pet if it hasn't expired
  }

  updateButtonStates(); // update state of 3 buttons. functions usually do this themselves, but we need to do it in the loop too.

  if (itemBeingPlaced != -1 && rightButtonState && startHandlingPlacing) {
    handleItemPlacing(); // if an item is being placed, sort it out
  }

  if (itemBeingPlaced != -1 && rightButtonState && handleFoodPlacing) {
    handleFoodPlacingLogic(); // if food is being placed, sort it out
  }

  if (uiTimer > 0) { //only draw UI elements if the UI timeout hasnt expired
    if (detectCursorTouch(0, 113, 128, 15)) { 
      uiTimer = 100; // reset UI timeout if cursor is touching bottom bar
    }

    uiTimer--; // tick UI timeout
    drawEmotionUI(); // draw pet status in top left corner

    drawLiveData(); // draw time and date at top of screen

    drawBottomUI(); // draw bottom bar and any other ui elements along with it (settings, shop, ect)
  }

  if (showPetMenu) { // only draw pet menu if pet is clicked
    drawPetMenu();
  }

  if (shouldDrawCursor) { // cursor disappears after no movement
    drawCursor();
  }

  display.display(); // final push to display buffer

  screenRecord(); // push display buffer to serial if enabled (use oled_viewer.py to view)

  audioEngine(); // write current tone to speaker pin

  handleSleepMode(); // check if switch is in off position, if it is, trigger sleep mode, and handle wake up

  delay(loopDelay);
}