#include "petLines.h"
#include <Arduino.h>

DRAM_ATTR String idleLines[] = {
  "whassup",
  "am good",
  "lmao",
  "uhhh",
  "beep beep im a sheep",
  "wheeyy",
  "heheheha",
  "what to say",
  "hello",
  "a red spy is in the base!",
  "protect the briefcase",
  "wahoo",
  "are you in class",
  "sudo rm -rf /",
  "gold gold gold",
  "gonna play GooseStrike 2",
  "fatty fatty",
  "x formerly twitter"
};

const size_t idleLinesCount = sizeof(idleLines) / sizeof(idleLines[0]);

DRAM_ATTR String beingCarriedLines[] = {
  "put me down!",
  "bro stop",
  "i hate this",
  "stop it",
  "omg why"
};

const size_t beingCarriedLinesCount = sizeof(beingCarriedLines) / sizeof(beingCarriedLines[0]);

DRAM_ATTR String hungryLines[] = {
  "stomach is a lil empty",
  "anything in the fridge?",
  "*rumble*",
  "food...",
  "hubgry :((",
  "feed?",
  "BRO FEED ME",
  "IM ACTUALLY GONNA DIE",
  "SERIOUSLY I WILL DIE",
  "GIVE ME FOOD.",
  "you suck man give me food",
  "FOOD. IN. MY. MOUTH."
};

const size_t hungryLinesCount = sizeof(hungryLines) / sizeof(hungryLines[0]);

DRAM_ATTR String boredLines[] = {
  "so bored",
  "play with me!",
  "give me attention",
  "please pong"
};

const size_t boredLinesCount = sizeof(boredLines) / sizeof(boredLines[0]);