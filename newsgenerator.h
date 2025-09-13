#ifndef NEWSGENERATOR_H
#define NEWSGENERATOR_H

#include <Arduino.h>
#include <RTClib.h>

extern RTC_DS3231 rtc;
extern const char* daysOfTheWeek[7];
extern const String gameNames[5];
extern int gameLibraryCount;
extern String displayNames[];
extern int bitmapCount;

int seededRandom(int max, unsigned long seed);
unsigned long mixSeed(unsigned long base, unsigned long salt);
String generateNewsHeadline(int seedModifier);

#endif