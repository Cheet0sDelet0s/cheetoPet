#ifndef EEPROMHANDLER_H
#define EEPROMHANDLER_H

#include <Arduino.h>
#include <vector>

struct ItemList {
  uint8_t type, x, y;
};

struct SaveGame {
  uint8_t hunger;
  uint8_t sleep;
  uint8_t fun;
  uint8_t poop;
  uint8_t money;
  uint8_t pongXP;
  uint8_t pongLVL;
  uint8_t invent[8];
  uint8_t inventItems;
  std::vector<ItemList> homePlot;
  std::vector<ItemList> outsidePlot;
  uint8_t foodInv[8];
  uint8_t foodInvItems;
  uint8_t saveVersion;
  uint8_t mouseMode;
  uint8_t petType;
  String petName;
};

// void writeStructEEPROM(uint16_t addr, SaveGame& savegame); // legacy save game
// SaveGame readStructEEPROM(uint16_t addr);

void eepromWriteByte(uint16_t addr, uint8_t data);
uint8_t eepromReadByte(uint16_t addr);
uint16_t saveVectorToEEPROM(uint16_t addr, const std::vector<ItemList> &vec);
uint16_t loadVectorFromEEPROM(uint16_t addr, std::vector<ItemList> &vec);
void eepromWriteString(uint16_t addr, const String &str);
String eepromReadString(uint16_t addr);

#endif