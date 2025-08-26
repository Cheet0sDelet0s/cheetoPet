#include "eepromHandler.h"
#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS 0x57 // Check your chip
#define EEPROM_SIZE 4096    // AT24C32 = 4KB

void writeStructEEPROM(uint16_t addr, SaveGame& savegame) {
  const byte* ptr = (const byte*)(const void*)&savegame;
  for (size_t i = 0; i < sizeof(SaveGame); i++) {
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