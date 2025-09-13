#ifndef NEWSGENERATOR_H
#define NEWSGENERATOR_H

#include <Arduino.h>
#include <RTClib.h>

extern RTC_DS3231 rtc;

int seededRandom(int max, int seed);
String generateNewsHeadline(int seedModifier);

#endif