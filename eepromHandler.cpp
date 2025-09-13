#include "eepromHandler.h"
#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS 0x57 
#define EEPROM_SIZE 4096    // AT24C32 = 4KB

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

// Write an Arduino String to EEPROM
void eepromWriteString(uint16_t addr, const String &str) {
  for (uint16_t i = 0; i < str.length(); i++) {
    eepromWriteByte(addr++, str[i]);
  }
  eepromWriteByte(addr, 0); // Null terminator
}

// Read a string from EEPROM into an Arduino String
String eepromReadString(uint16_t addr) {
  String result = "";
  uint8_t c;

  while (true) {
    c = eepromReadByte(addr++);
    if (c == 0 || c == 0xFF) { // Stop at null terminator or invalid read
      break;
    }
    result += (char)c;
  }

  return result;
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