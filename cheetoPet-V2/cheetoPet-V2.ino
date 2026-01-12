 /***********************************\
 | cheetoPet V2 - by olly jeffery :) |
 \***********************************/

// ---- LIBRARIES ----
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <RTClib.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_ADS1X15.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "vector"
#include "Picopixel.h"

// ---- CHEETOPET FILES ----
#include "hardwareConfig.h"
#include "hardware.h"
#include "graphics.h"

/* ---- INFO ----
  FLASH AND DRAM:
    anything that is read/written to infrequently should go in flash. (like storage of computer)
    anything that is used quite a bit goes in dram (like ram of computer)
    this makes things run faster!
                                          v  v  v
    to put a variable in DRAM, use this: DRAM_ATTR int myVariable = 0;
*/

void setup() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(CHRG_PIN, INPUT_PULLUP);
  pinMode(STDBY_PIN, INPUT_PULLUP);
  pinMode(A_BUTTON, INPUT_PULLUP);
  pinMode(B_BUTTON, INPUT_PULLUP);
  pinMode(X_BUTTON, INPUT_PULLUP);
  pinMode(SPKR_PIN, OUTPUT);

  ledcAttach(SPKR_PIN, 5000, 8);
  ledcWriteTone(SPKR_PIN, 0);     // start silent

  Serial.begin(921600);
  Serial.println("begun serial at 921600 baudrate! hello world!");
  analogReadResolution(12);

  Wire.begin(SDA_ALT, SCL_ALT);

  Serial.println("begun i2c! beginning to initialise peripherals...");

  initDisplay();
}

void loop() {

}