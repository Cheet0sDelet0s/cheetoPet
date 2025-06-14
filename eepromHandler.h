#ifndef EEPROMHANDLER_H
#define EEPROMHANDLER_H

#include <Arduino.h>

extern void writeEEPROM(uint16_t addr, byte data);
extern byte readEEPROM(uint16_t addr);
extern struct SaveGame;

#endif