#ifndef PET_H
#define PET_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>
#include <vector>

// ---- CHEETOPET FILES ----
#include <hardware.h>
#include <bitmaps.h>
#include <behaviourtree.h>
#include <petLines.h>

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

DRAM_ATTR extern String currentPetMessage;
DRAM_ATTR extern int messageDisplayTime;
DRAM_ATTR extern int messageMaxTime;

// PET MOVEMENT
DRAM_ATTR inline int petX = 64;
DRAM_ATTR inline int petY = 32;

DRAM_ATTR inline int petMoveX = 64;
DRAM_ATTR inline int petMoveY = 32;
DRAM_ATTR inline int petMoveSpeed = 0;
DRAM_ATTR inline bool movePet = false;
DRAM_ATTR inline int petMoveAnim = 0;
DRAM_ATTR inline int petDir = 1;
DRAM_ATTR inline int petSitTimer = 0;
DRAM_ATTR inline int petSitType = 0;
DRAM_ATTR inline int petStatus = 0;

//TIMING
inline const long interval = 50; // interval for pet movement? i think??
inline unsigned long previousMillis = 0; // previous millis() value

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


#endif