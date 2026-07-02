/* ---- hardware.h ----

  holds all functions for hardware-software interaction - like a kernel!! linus torvalds reference??? terry davis reference??????
  stuff like perihperal initialisation, getting gyro values, etc

*/

#ifndef HARDWARE_H
#define HARDWARE_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <RTClib.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_ADS1X15.h>
#include <at24c04.h>

// include pin configurations
#include "hardwareConfig.h"
#include "graphics.h"
#include "pet.h"

extern Adafruit_SH1107 display;
extern Adafruit_ADS1115 ads;
extern RTC_DS3231 rtc;
extern DateTime tempDateTime;
extern MPU9250_asukiaaa mpu;
extern AT24C04 eepromChip;

//buttons
extern DRAM_ATTR bool leftButtonState;
extern DRAM_ATTR bool middleButtonState;
extern DRAM_ATTR bool rightButtonState;
extern DRAM_ATTR bool powerSwitchState;
extern DRAM_ATTR bool previousLeftState;
extern DRAM_ATTR bool previousMiddleState;
extern DRAM_ATTR bool previousRightState;

//gyro
DRAM_ATTR inline float totalG = 0.0;
DRAM_ATTR inline float angleX;
DRAM_ATTR inline float angleY;
DRAM_ATTR inline float angleZ;
DRAM_ATTR inline float posX;
DRAM_ATTR inline float posY;
DRAM_ATTR inline float posZ;

inline float gyroXOffset = 0;
inline float gyroYOffset = 0;
inline float gyroZOffset = 0;
inline float accelXOffset = 0;
inline float accelYOffset = 0;
inline float accelZOffset = 0;

DRAM_ATTR inline float currentBatteryVoltage = 3.7;
DRAM_ATTR inline int currentBatteryPercentage = 100;

DRAM_ATTR inline unsigned long lastUpdate = 0;

extern int numSongs;

struct Note {
  float freq; // hz
  uint16_t length; // ms
};

// Song metadata wrapper
struct Song {
  Note* notes;
  int length;
};

extern Song songs[];

void initDisplay();
void initPins();
void initPeripherals();
void batteryManagement();
void lightSleep(int type);
int getBatteryPercentage();
float getBatteryVoltage();
int getBatteryStatus();
void bootMenu();
void dumpBufferASCII();
void screenRecord();
void waitForSelectRelease();
void updateButtonStates();
void updateGyro();
void playSong(Song song);
void playRandomSong();
void audioEngine();
void priorityQueueTone(float freq, int length);
void queueTone(float freq, uint16_t length);
void clearTones();
void updatePreviousStates();
bool buttonPressedThisFrame(int num);
void calibrateGyro();

void eepromWriteByte(uint16_t addr, uint8_t data);
uint8_t eepromReadByte(uint16_t addr);
uint16_t saveVectorToEEPROM(uint16_t addr, const std::vector<ItemList> &vec);
uint16_t loadVectorFromEEPROM(uint16_t addr, std::vector<ItemList> &vec);
void eepromWriteString(uint16_t addr, const char *str);
void eepromReadString(uint16_t addr, char *buffer, uint16_t bufferSize);

#endif
