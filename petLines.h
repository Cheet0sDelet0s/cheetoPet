#ifndef PETLINES_H
#define PETLINES_H

#include <Arduino.h>

extern String idleLines[];
extern const size_t idleLinesCount;

extern String beingCarriedLines[];
extern const size_t beingCarriedLinesCount;

extern String hungryLines[];
extern const size_t hungryLinesCount;

extern String boredLines[];
extern const size_t boredLinesCount;

extern String tiredLines[];
extern const size_t tiredLinesCount;

extern String shakenLines[];
extern const size_t shakenLinesCount;

extern String generateSentence();

#endif