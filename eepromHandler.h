#ifndef EEPROMHANDLER_H
#define EEPROMHANDLER_H

#include <Arduino.h>

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
  uint8_t saveVersion;
};

void writeStructEEPROM(uint16_t addr, SaveGame& savegame);
SaveGame readStructEEPROM(uint16_t addr);


#endif