#include "eepromHandler.h"
#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS 0x57 // Check your chip
#define EEPROM_SIZE 4096    // AT24C32 = 4KB

struct SaveGame {
  uint8_t hunger;
  uint8_t sleep;
  uint8_t fun;
  uint8_t money;
  uint8_t pongXP;
  uint8_t pongLVL;
  uint8_t invent[8];
  uint8_t inventItems;
  uint8_t placed[30];
  uint8_t placedItems;
  uint8_t placedX[30];
  uint8_t placedY[30];
  uint8_t foodInv[8];
  uint8_t foodInvItems;
};

void writeStructEEPROM(uint16_t addr, const Config& config) {
  const byte* ptr = (const byte*)(const void*)&config;
  for (size_t i = 0; i < sizeof(Config); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((addr + i) >> 8);    // MSB
    Wire.write((addr + i) & 0xFF);  // LSB
    Wire.write(ptr[i]);
    Wire.endTransmission();
    delay(5); // Required write delay
  }
}

SaveGame readStructEEPROM(uint16_t addr) {
  SaveGame savegame;
  byte* ptr = (byte*)(void*)&savegame;
  for (size_t i = 0; i < sizeof(SaveGame); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((addr + i) >> 8);    // MSB
    Wire.write((addr + i) & 0xFF);  // LSB
    Wire.endTransmission();

    Wire.requestFrom(EEPROM_ADDRESS, 1);
    if (Wire.available()) {
      ptr[i] = Wire.read();
    } else {
      ptr[i] = 0xFF; // Default/fail value
    }
  }
  return savegame;
}