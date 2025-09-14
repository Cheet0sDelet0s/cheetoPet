#include "eepromHandler.h"
#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS 0x57 
#define EEPROM_SIZE 4096    // AT24C32 = 4KB
#define MAX_STRING_LENGTH 128  // Maximum string length to read/write

void eepromWriteByte(uint16_t addr, uint8_t data) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((addr >> 8) & 0xFF); // MSB
  Wire.write(addr & 0xFF);        // LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(5); // EEPROM write cycle
}

uint8_t eepromReadByte(uint16_t addr) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((addr >> 8) & 0xFF);
  Wire.write(addr & 0xFF);
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0xFF; // Default on fail
}

// Write a C-style string to EEPROM
void eepromWriteString(uint16_t addr, const char *str) {
  uint16_t i = 0;
  while (str[i] != '\0' && i < MAX_STRING_LENGTH - 1) {
    eepromWriteByte(addr + i, str[i]);
    i++;
  }
  eepromWriteByte(addr + i, 0); // Null terminator
}

// Read a C-style string from EEPROM into a buffer
// The buffer must have at least MAX_STRING_LENGTH bytes
void eepromReadString(uint16_t addr, char *buffer, uint16_t bufferSize) {
  uint16_t i = 0;
  uint8_t c;

  while (i < bufferSize - 1 && i < MAX_STRING_LENGTH - 1) {
    c = eepromReadByte(addr + i);
    if (c == 0 || c == 0xFF) {
      break; // Stop at null terminator or invalid read
    }
    buffer[i] = (char)c;
    i++;
  }
  buffer[i] = '\0'; // Ensure string is null-terminated
}


uint16_t saveVectorToEEPROM(uint16_t addr, const std::vector<ItemList> &vec) {
  eepromWriteByte(addr++, vec.size()); // Save size
  for (const auto &item : vec) {
    eepromWriteByte(addr++, item.type);
    eepromWriteByte(addr++, item.x);
    eepromWriteByte(addr++, item.y);
  }
  return addr;
}

uint16_t loadVectorFromEEPROM(uint16_t addr, std::vector<ItemList> &vec) {
  vec.clear();
  uint8_t size = eepromReadByte(addr++);
  for (int i = 0; i < size; i++) {
    ItemList item;
    item.type = eepromReadByte(addr++);
    item.x = eepromReadByte(addr++);
    item.y = eepromReadByte(addr++);
    vec.push_back(item);
  }
  return addr;
}

// void writeStructEEPROM(uint16_t addr, SaveGame& savegame) {  // legacy save game
//   const byte* ptr = (const byte*)(const void*)&savegame;
//   for (size_t i = 0; i < sizeof(SaveGame); i++) {
//     Wire.beginTransmission(EEPROM_ADDRESS);
//     Wire.write((addr + i) >> 8);    // MSB
//     Wire.write((addr + i) & 0xFF);  // LSB
//     Wire.write(ptr[i]);
//     Wire.endTransmission();
//     delay(5); // Required write delay
//   }
// }

// SaveGame readStructEEPROM(uint16_t addr) {
//   SaveGame savegame;
//   byte* ptr = (byte*)(void*)&savegame;
//   for (size_t i = 0; i < sizeof(SaveGame); i++) {
//     Wire.beginTransmission(EEPROM_ADDRESS);
//     Wire.write((addr + i) >> 8);    // MSB
//     Wire.write((addr + i) & 0xFF);  // LSB
//     Wire.endTransmission();

//     Wire.requestFrom(EEPROM_ADDRESS, 1);
//     if (Wire.available()) {
//       ptr[i] = Wire.read();
//     } else {
//       ptr[i] = 0xFF; // Default/fail value
//     }
//   }
//   return savegame;
// }