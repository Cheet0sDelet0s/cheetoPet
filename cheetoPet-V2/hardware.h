#ifndef HARDWARE_H
#define HARDWARE_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <RTClib.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_ADS1X15.h>

// include pin configurations
#include "hardwareConfig.h"

extern Adafruit_SH1107 display;
extern Adafruit_ADS1115 ads;
extern RTC_DS3231 rtc;
extern DateTime tempDateTime;
extern MPU9250_asukiaaa mpu;

void initDisplay();

#endif
