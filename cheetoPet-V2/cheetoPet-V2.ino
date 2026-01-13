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
  initPins(); // set pinModes
  initPeripherals(); // begin serial, i2c, etc
  initDisplay(); // initialise display, clear it, set text color to white (i always forget to do this).
  
}

void loop() {

}