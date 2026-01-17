#ifndef PET_H
#define PET_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>
#include <vector>

// ---- CHEETOPET FILES ----
#include <hardware.h>
#include <graphics.h>
#include <behaviourtree.h>
#include <petLines.h>
#include <os.h>
#include <bitmaps.h>

#pragma once

// PET VARIABLES
inline String petName = "";
inline int petHunger = 50;
inline int petSleep = 50;
inline int petFun = 50;
inline int petPoop = 50;
inline float petMood = 50;
inline float money = 40.00;
inline int userPet = 0;

inline String currentPetMessage = "";
inline int messageDisplayTime = 0;
inline int messageMaxTime = 0;

// PET MOVEMENT
DRAM_ATTR inline int petX = 64;
DRAM_ATTR inline int petY = 32;

DRAM_ATTR inline int petMoveX = 64;
DRAM_ATTR inline int petMoveY = 32;
inline int petMoveSpeed = 0;
inline bool movePet = false;
inline int petMoveAnim = 0;
inline int petDir = 1;
inline int petSitTimer = 0;
inline int petSitType = 0;
inline int petStatus = 0;
inline bool movingPet = false;

inline int currentArea = 0;

inline int placedFood[10] = {};
inline int amountFoodPlaced = 0;
inline int placedFoodX[10] = {};
inline int placedFoodY[10] = {};

inline std::vector<ItemList>* currentAreaPtr = nullptr;

inline int inventory[16] = {};
inline int inventoryItems = 0;

inline int itemToPackUp = -1;

//TIMING
inline const long interval = 50; // interval for pet movement? i think??
DRAM_ATTR inline unsigned long previousMillis = 0; // previous millis() value

struct Door {
  uint8_t x, y, area, type;
};

struct Pet {
  String name;
  int stillID; // bitmap for standing still
  int walk1ID; // bitmap for walking animation frame 1
  int walk2ID; // bitmap for walking animation frame 2
  int sitID;   // bitmap for sitting
  int marshmellowID;  // bitmap for marshmellow roasting
};

const inline Pet pets[] = {
  {"goose", 1, 21, 23, 26, 28},
  {"hedgehog", 43, 45, 47, 51, 49},
  {"bird", 53, 55, 57, 59, 61}
};

struct SaveGame {
  uint8_t hunger;
  uint8_t sleep;
  uint8_t fun;
  uint8_t poop;
  uint8_t money;
  uint8_t invent[8];
  uint8_t inventItems;
  std::vector<ItemList> homePlot;
  std::vector<ItemList> outsidePlot;
  uint8_t foodInv[8];
  uint8_t foodInvItems;
  uint8_t saveVersion;
  uint8_t petType;
  uint8_t namePrefix;
  uint8_t nameSuffix;
  uint8_t games[8];
  uint8_t gameCount;
};

void killPet(String deathReason);
void updatePetMovement();
void startMovingPet(int x, int y, int speed);
void petMessage(String message);
bool checkItemIsPlaced(int item);
int findIndexByType(const std::vector<ItemList>& vec, uint8_t type);
void sitPet(int time, int type);
void updateAreaPointers();
void handlePetButtons();
void purchaseItem(int id, float price);
void placeItemFromInventory(int id);
void handleItemPlacing();

void updatePet();
void drawPetHome();

#endif