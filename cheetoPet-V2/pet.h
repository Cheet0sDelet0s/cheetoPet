#ifndef PET_H
#define PET_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ---- CHEETOPET FILES ----
#include <hardware.h>
#include <bitmaps.h>

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
  uint8_t namePrefix;
  uint8_t nameSuffix;
  uint8_t games[8];
  uint8_t gameCount;
  float gyroXOffset;
  float gyroYOffset;
  float gyroZOffset;
};

#endif