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
DRAM_ATTR int inventory[10] = { 3, 5, 4, 6 };
DRAM_ATTR int inventoryItems = 4;
DRAM_ATTR int placedHomeItems[10] = { 7 };
DRAM_ATTR int amountItemsPlaced = 1;
DRAM_ATTR int placedHomeItemsX[10] = { 98 };
DRAM_ATTR int placedHomeItemsY[10] = { 4 };
DRAM_ATTR int itemBeingPlaced = -1;
DRAM_ATTR bool startHandlingPlacing = false;

//food inventory
DRAM_ATTR int foodInventory[10] = { 18 };
DRAM_ATTR int foodInventoryItems = 1;
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

DRAM_ATTR int petHunger = 49;
DRAM_ATTR int petFun = 50;
DRAM_ATTR int petSleep = 50;
DRAM_ATTR int petX = 64;
DRAM_ATTR int petY = 32;
DRAM_ATTR bool showPetMenu = false;
DRAM_ATTR bool movingPet = false;

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
  messageMaxTime = currentPetMessage.length() * 20;
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

//BITMAPS
const unsigned char pet_gooseStill[] PROGMEM = {
  // 'goose still, 12x15px
  0x07, 0xc0, 0x0a, 0xa0, 0x08, 0x30, 0x09, 0xc0, 0x11, 0x00, 0x65, 0x00, 0x8b, 0x00, 0x93, 0x00,
  0x8d, 0x00, 0x81, 0x00, 0x7e, 0x00, 0x42, 0x00, 0x42, 0x00, 0xc6, 0x00, 0xe7  //, 0x00
};

const unsigned char pet_gooseStillBig[] PROGMEM = {
  // 'gooseStillBig, 16x26px
  0x00, 0xfc, 0x03, 0x2a, 0x04, 0x03, 0x04, 0x1c, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20,
  0x7b, 0x20, 0x84, 0xa0, 0x88, 0xa0, 0x88, 0xa0, 0x87, 0x20, 0x80, 0x20, 0x80, 0x20, 0x7f, 0xc0,
  0x20, 0x40, 0x20, 0x40, 0x20, 0x40, 0x20, 0x40, 0x70, 0xe0, 0x50, 0xa0, 0x50, 0xa0, 0x58, 0xb0,
  0x44, 0x88, 0x38, 0x70
};

const unsigned char pet_gooseStillBigMask[] PROGMEM = {
  // 'gooseStillBigMask, 17x26px
  0x01, 0x81, 0x80, 0x02, 0x6a, 0x80, 0x05, 0xfe, 0x00, 0x05, 0xf1, 0x80, 0x05, 0xee, 0x00, 0x05,
  0xe8, 0x00, 0x05, 0xe8, 0x00, 0x3d, 0xe8, 0x00, 0x42, 0x68, 0x00, 0xbd, 0xa8, 0x00, 0xbb, 0xa8,
  0x00, 0xbb, 0xa8, 0x00, 0xbc, 0x68, 0x00, 0xbf, 0xe8, 0x00, 0xbf, 0xe8, 0x00, 0x40, 0x10, 0x00,
  0x2f, 0xd0, 0x00, 0x28, 0x50, 0x00, 0x28, 0x50, 0x00, 0x28, 0x50, 0x00, 0x44, 0x88, 0x00, 0x54,
  0xa8, 0x00, 0x54, 0xa8, 0x00, 0x52, 0xa4, 0x00, 0x5d, 0xba, 0x00, 0x42, 0x84, 0x00
};

const unsigned char ui_couch1[] PROGMEM = {
  // 'couch1, 28x30px
  0x01, 0x11, 0x11, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x15, 0x55, 0x55, 0x40, 0x1a, 0xaa, 0xaa, 0x80,
  0x15, 0x55, 0x55, 0x00, 0x0a, 0xaa, 0xaa, 0x80, 0x15, 0x55, 0x55, 0x40, 0x0a, 0xaa, 0xaa, 0x80,
  0x15, 0x55, 0x55, 0x00, 0x2a, 0xaa, 0xa8, 0x80, 0x55, 0x55, 0x55, 0x40, 0x7c, 0xaa, 0xa3, 0xe0,
  0x55, 0x55, 0x51, 0x50, 0x38, 0x00, 0x03, 0xa0, 0x55, 0x55, 0x55, 0x40, 0x7c, 0xff, 0xfb, 0xe0,
  0x55, 0x55, 0x53, 0x70, 0x29, 0xbb, 0xba, 0xa0, 0x55, 0xdd, 0xdd, 0x40, 0x29, 0xaa, 0xaa, 0x80,
  0x15, 0x55, 0x51, 0x40, 0x2a, 0xaa, 0xa2, 0x80, 0x55, 0x55, 0x55, 0x40, 0x28, 0x00, 0x02, 0x80,
  0x15, 0x55, 0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x40, 0x00, 0x00, 0x00, 0x00,
  0x14, 0x00, 0x01, 0x00, 0x22, 0x00, 0x00, 0x00
};

const unsigned char ui_table[] PROGMEM = {
  // table, 18x14px
  0x3f, 0xff, 0x00, 0x40, 0x00, 0x80, 0x40, 0x00, 0x80, 0x76, 0xdb, 0x80, 0x40, 0x00, 0x80, 0x80,
  0x00, 0x40, 0xdb, 0x6d, 0xc0, 0x80, 0x00, 0x40, 0x80, 0x00, 0x40, 0x7f, 0xff, 0x80, 0x28, 0x05,
  0x00, 0x28, 0x05, 0x00, 0x28, 0x05, 0x00, 0x38, 0x07, 0x00
};

const unsigned char ui_fireplace[] PROGMEM = {
  // 'fireplace, 15x15px
  0xff, 0xfe, 0x80, 0x02, 0x80, 0x02, 0xff, 0xfe, 0x60, 0x0c, 0x3f, 0xf8, 0x20, 0x08, 0x26, 0x48,
  0x2e, 0xe8, 0x27, 0xe8, 0x27, 0xe8, 0x2f, 0xe8, 0x7f, 0xfc, 0xc0, 0x06, 0xff, 0xfe
};

// 'bigTable', 46x38px
const unsigned char ui_bigTable[] PROGMEM = {
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff,
  0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff,
  0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff,
  0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdc, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x04, 0x55, 0x55, 0x55, 0x55, 0x55, 0x50, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x7d, 0x55,
  0x55, 0x55, 0x55, 0xf8, 0x6e, 0xaa, 0xaa, 0xaa, 0xaa, 0xd8, 0x74, 0x00, 0x00, 0x00, 0x00, 0xe8,
  0x6c, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x74, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x6c, 0x00, 0x00, 0x00,
  0x00, 0xd8, 0x74, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x6c, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x7c, 0x00,
  0x00, 0x00, 0x00, 0xf8
};

const unsigned char ui_window[] PROGMEM = {
  // 'window, 29x27px
  0x0f, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xc0, 0x18, 0x07, 0x00, 0xc0, 0x19, 0x27, 0x20, 0xc0,
  0x1a, 0x47, 0x48, 0xc0, 0x18, 0x87, 0x10, 0xc0, 0x19, 0x07, 0x20, 0xc0, 0x1a, 0x07, 0x00, 0xc0,
  0x18, 0x07, 0x00, 0xc0, 0x18, 0x07, 0x00, 0xc0, 0x18, 0x07, 0x00, 0xc0, 0x1f, 0xff, 0xff, 0xc0,
  0x18, 0x07, 0x00, 0xc0, 0x1a, 0x07, 0x48, 0xc0, 0x18, 0x87, 0x00, 0xc0, 0x19, 0x07, 0x20, 0xc0,
  0x18, 0x07, 0x00, 0xc0, 0x18, 0x07, 0x00, 0xc0, 0x18, 0x07, 0x00, 0xc0, 0x18, 0x07, 0x00, 0xc0,
  0x18, 0x07, 0x00, 0xc0, 0x1f, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0x80, 0xf0, 0x00, 0x00, 0x78,
  0xff, 0xff, 0xff, 0xf8, 0xaa, 0xaa, 0xaa, 0xa8, 0xff, 0xff, 0xff, 0xf8
};

// 'menu', 12x9px
const unsigned char ui_menu[] PROGMEM = {
  0xdf, 0xe0, 0xdf, 0xe0, 0x00, 0x00, 0xdf, 0xe0, 0xdf, 0xe0, 0xdf, 0xe0, 0x00, 0x00, 0xdf, 0xe0,
  0xdf, 0xe0, 0x00, 0x00, 0x00, 0x00
};
// 'pencil', 11x11px
const unsigned char ui_pencil[] PROGMEM = {
  0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x0e, 0x20, 0x17, 0x40, 0x23, 0x80, 0x41, 0x00, 0x82, 0x00,
  0xc4, 0x00, 0xe8, 0x00, 0xf0, 0x00
};
// 'settings', 11x11px
const unsigned char ui_settings[] PROGMEM = {
  0x0a, 0x00, 0x4e, 0x40, 0x3f, 0x80, 0x31, 0x80, 0xe0, 0xe0, 0x64, 0xc0, 0xe0, 0xe0, 0x31, 0x80,
  0x3f, 0x80, 0x4e, 0x40, 0x0a, 0x00
};

// 'shop', 16x11px
const unsigned char ui_shop[] PROGMEM = {
  0xc0, 0x00, 0xe0, 0x00, 0x70, 0x00, 0x3f, 0xff, 0x24, 0x93, 0x1f, 0xfe, 0x12, 0x4c, 0x0f, 0xf8,
  0x09, 0x30
};
// 'back', 9x8px
const unsigned char ui_back[] PROGMEM = {
  0x38, 0x00, 0x70, 0x00, 0xfe, 0x00, 0x71, 0x00, 0x38, 0x80, 0x00, 0x80, 0x01, 0x00, 0x3e, 0x00,
  0x00, 0x00
};
// 'inventory', 16x9px
const unsigned char ui_inventory[] PROGMEM = {
  0x3f, 0xfc, 0x40, 0xe2, 0x81, 0x51, 0xfe, 0x4f, 0x20, 0x44, 0x20, 0x44, 0x2a, 0x44, 0x20, 0x44,
  0x3f, 0xfc
};

const unsigned char ui_cursor[] PROGMEM = {
  // 'cursor, 9x11px
  0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00, 0x78, 0x00, 0x7c, 0x00, 0x56, 0x00, 0x57, 0x00,
  0x7e, 0x00, 0x70, 0x00, 0x00, 0x00
};

const unsigned char ui_cursorMask[] PROGMEM = {
  // 'cursorMask, 11x13px
  0xf8, 0x00, 0xfc, 0x00, 0xde, 0x00, 0xcf, 0x00, 0xc7, 0x80, 0xc3, 0xc0, 0xc1, 0xe0, 0xd4, 0xe0,
  0xd4, 0x60, 0xc0, 0xe0, 0xc7, 0xe0, 0xff, 0xc0, 0xfe, 0x00
};

const unsigned char item_apple[] PROGMEM = {
  // 'apple, 11x11px
  0x03, 0x80, 0x04, 0x00, 0x35, 0x80, 0x4e, 0x40, 0x80, 0x20, 0x88, 0x20, 0x90, 0x20, 0x40, 0x40,
  0x40, 0x40, 0x31, 0x80, 0x0e, 0x00
};

const unsigned char pet_gooseSleep[] PROGMEM = {
  // 'gooseSleep, 24x39px
  0x00, 0x00, 0x3f, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x30, 0x00,
  0x00, 0x20, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x80, 0x00, 0x03,
  0x00, 0x00, 0x04, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x04, 0x0c, 0x00,
  0x05, 0xb4, 0x00, 0x04, 0x04, 0x00, 0x04, 0x34, 0x00, 0x04, 0x2c, 0x00, 0x04, 0x24, 0x00, 0x7b,
  0x20, 0x00, 0x84, 0xa0, 0x00, 0x88, 0xa0, 0x00, 0x88, 0xa0, 0x00, 0x87, 0x20, 0x00, 0x80, 0x20,
  0x00, 0x80, 0x20, 0x00, 0x7f, 0xc0, 0x00, 0x20, 0x40, 0x00, 0x20, 0x40, 0x00, 0x20, 0x40, 0x00,
  0x20, 0x40, 0x00, 0x70, 0xe0, 0x00, 0x50, 0xa0, 0x00, 0x50, 0xa0, 0x00, 0x58, 0xb0, 0x00, 0x44,
  0x88, 0x00, 0x38, 0x70, 0x00
};

const unsigned char item_banana[] PROGMEM = {
  // 'banana, 10x11px
  0x08, 0x00, 0x1c, 0x00, 0x38, 0x00, 0x50, 0x00, 0x90, 0x00, 0x90, 0x00, 0x88, 0x00, 0x86, 0x00,
  0x61, 0xc0, 0x10, 0x40, 0x0f, 0x80
};

struct BitmapInfo {
  const uint8_t* data;
  uint16_t width;
  uint16_t height;
};

const BitmapInfo bitmaps[] = {
  { pet_gooseStill, 12, 15 },         //0
  { pet_gooseStillBig, 16, 26 },      //1
  { pet_gooseStillBigMask, 17, 26 },  //2
  { ui_couch1, 28, 30 },              //3
  { ui_table, 18, 14 },               //4
  { ui_fireplace, 15, 15 },           //5
  { ui_bigTable, 46, 38 },            //6
  { ui_window, 29, 27 },              //7
  { ui_menu, 12, 9 },                 //8
  { ui_pencil, 11, 11 },              //9
  { ui_settings, 11, 11 },            //10
  { ui_shop, 16, 11 },                //11
  { ui_back, 9, 8 },                  //12
  { ui_inventory, 16, 9 },            //13
  { ui_cursor, 9, 11 },               //14
  { ui_cursorMask, 11, 13 },          //15
  { item_apple, 11, 11 },             //16
  { pet_gooseSleep, 24, 39 },         //17
  { item_banana, 10, 11 }             //18
};

String displayNames[] = {
  { "goose" },          //0
  { "goose" },          //1
  { "goose" },          //2
  { "couch" },          //3
  { "table" },          //4
  { "fireplace" },      //5
  { "big table" },      //6
  { "window" },         //7
  { "menuIcon" },       //8
  { "pencilIcon" },     //9
  { "settingsIcon" },   //10
  { "shopIcon" },       //11
  { "backIcon" },       //12
  { "inventoryIcon" },  //13
  { "cursorIcon" },     //14
  { "cursorMask" },     //15
  { "apple" },          //16
  { "goose" },          //17
  { "banana" }          //18
};

String idleLines[] = {
  "whassup",
  "am good",
  "lmao",
  "uhhh",
  "beep beep im a sheep",
  "wheeyy",
  "heheheha",
  "what to say",
  "hello",
  "a red spy is in the base"
};

String beingCarriedLines[] = {
  "put me down!",
  "bro stop",
  "i hate this",
  "stop it",
  "omg why"
};

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
  display.print("/10");

  for (int i = 0; i < inventoryItems; i++) {
    int itemX = 4;
    int itemY = 76 + (i * 8);
    int itemW = 60;  // You can fine-tune this depending on longest item length
    int itemH = 8;

    bool hovered = detectCursorTouch(itemX, itemY, itemW, itemH);

    if (hovered) {
      // Highlight the item with inverse color
      //display.fillRect(itemX, itemY, itemW, itemH, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK, SH110X_WHITE);
      if (rightButtonState) {
        if (itemBeingPlaced == -1) {  //start placing item
          itemBeingPlaced = inventory[i];
          waitForSelectRelease();
          Serial.println("starting placement handling");
          Serial.print("set itemBeingPlaced to: ");
          Serial.println(itemBeingPlaced);
          startHandlingPlacing = true;
        }
      }
    } else {
      display.setTextColor(SH110X_WHITE);
    }

    display.setCursor(itemX, itemY);
    display.println(displayNames[inventory[i]]);
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
  display.print("/10");

  for (int i = 0; i < foodInventoryItems; i++) {
    int itemX = 4;
    int itemY = 76 + (i * 8);
    int itemW = 60;  // You can fine-tune this depending on longest item length
    int itemH = 8;

    bool hovered = detectCursorTouch(itemX, itemY, itemW, itemH);

    if (hovered) {
      // Highlight the item with inverse color
      //display.fillRect(itemX, itemY, itemW, itemH, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK, SH110X_WHITE);
      if (rightButtonState) {
        if (itemBeingPlaced == -1) {  //start placing item
          itemBeingPlaced = foodInventory[i];
          waitForSelectRelease();
          Serial.println("starting placement handling");
          Serial.print("set itemBeingPlaced to: ");
          Serial.println(itemBeingPlaced);
          handleFoodPlacing = true;
        }
      }
    } else {
      display.setTextColor(SH110X_WHITE);
    }

    display.setCursor(itemX, itemY);
    display.println(displayNames[foodInventory[i]]);
  }
}

void drawEmotionUI() {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("HUN: ");
  display.println(petHunger);
  display.print("FUN: ");
  display.println(petFun);
  display.print("SLP: ");
  display.println(petSleep);
}

void drawHomeItems() {
  display.drawFastHLine(0, 42, 127, SH110X_WHITE);

  for (int i = 0; i < amountItemsPlaced; i++) {
    const BitmapInfo& bmp = bitmaps[placedHomeItems[i]];
    display.drawBitmap(placedHomeItemsX[i], placedHomeItemsY[i], bmp.data, bmp.width, bmp.height, SH110X_WHITE);
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
  addToList(placedHomeItemsY, placedCount, 10, cursorY);

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
    display.print(batteryPercentage); display.print("%");
    display.setCursor(55, 10);
    display.print(batteryVoltage); display.print("v");
  }
  
  liveDataTimer++;
  if (liveDataTimer > 200) {liveDataTimer = 0; setBatteryLevel();}
}

void setPetBehaviour() {
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
  const int maxCharsPerLine = 7;  // Adjust based on display width
  const int charWidth = 6;
  const int lineHeight = 8;

  // 1. Split the message into lines
  std::vector<String> lines;
  for (int i = 0; i < currentPetMessage.length(); i += maxCharsPerLine) {
    lines.push_back(currentPetMessage.substring(i, i + maxCharsPerLine));
  }

  int numLines = lines.size();
  int bubbleWidth = min(maxCharsPerLine, (int)currentPetMessage.length()) * charWidth + 4;
  int bubbleHeight = numLines * lineHeight + 4;

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

  // 2. Draw the bubble
  int topLeftX = (bubbleDirection == 1) ? bubbleX : bubbleX - bubbleWidth;
  int topLeftY = bubbleY - bubbleHeight;

  display.drawRect(topLeftX, topLeftY, bubbleWidth, bubbleHeight, SH110X_WHITE);

  // Tail
  display.drawLine(bubbleX - 2 * bubbleDirection, bubbleY + 2, bubbleX - 2 * bubbleDirection, bubbleY, SH110X_WHITE);
  display.drawLine(bubbleX - 2 * bubbleDirection, bubbleY + 2, bubbleX, bubbleY - 2, SH110X_WHITE);
  //display.drawFastHLine(bubbleX + 5 * bubbleDirection, bubbleY - 5, -5 * bubbleDirection, SH110X_WHITE);

  // 3. Draw each line of text
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  for (int i = 0; i < numLines; i++) {
    int cursorX = topLeftX + 2;
    int cursorY = topLeftY + 2 + i * lineHeight;
    display.setCursor(cursorX, cursorY);
    display.print(lines[i]);
  }
  display.setTextColor(SH110X_WHITE);

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
    petMessage("am hungry");
    return SUCCESS;
  } 
};

class Idle : public Node {
public:
  NodeStatus tick() override {
    //1% chance to yap
    if (random(0, 100) == 1) {
      int messageRandomiser = random(0, 10);
      petMessage(idleLines[messageRandomiser]);
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
      int messageRandomiser = random(0, 6);
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
    int lastFoodY = placedFoodY[lastFoodIndex];

    if (lastFoodX == petX && lastFoodY == petY) {
      removeFromList(placedFood, lastFoodIndex, lastFoodIndex);
      removeFromList(placedFoodX, lastFoodIndex, lastFoodIndex);
      removeFromList(placedFoodY, lastFoodIndex, lastFoodIndex);
      amountFoodPlaced--;
      petHunger += 30;
      movePet = false;
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
    cursorY = constrain(angleY + 64, 0, 126);
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

  // if (rightButtonState) {
  //   petMessage("hello world");
  // }

  drawEmotionUI();

  drawLiveData();

  drawBottomUI();

  updateButtonStates();
  //Serial.println(startHandlingPlacing);

  if (itemBeingPlaced != -1 && rightButtonState && startHandlingPlacing) {
    handleItemPlacing();
  }

  if (itemBeingPlaced != -1 && rightButtonState && handleFoodPlacing) {
    handleFoodPlacingLogic();
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
    esp_deep_sleep_enable_gpio_wakeup(1 << SWITCH_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);  // enable waking up from deep sleep when switch is turned on / pulled high absoulute nerd

    esp_light_sleep_start();  //yoo he said the thing

    mpu9250_wake();
    DateTime now = rtc.now();

    TimeSpan timeSinceSlept = now - timeWhenSlept;

    int32_t seconds = timeSinceSlept.totalseconds();
    int32_t minutesSinceSlept = seconds / 60;

    petSleep += minutesSinceSlept;
    constrain(petSleep, 0, 100);

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
    const BitmapInfo& bmp2 = bitmaps[1];
    display.drawBitmap(55, 82, bmp2.data, bmp2.width, bmp2.height, SH110X_WHITE);
    display.display();
    delay(1000);
  }

  delay(5);
}