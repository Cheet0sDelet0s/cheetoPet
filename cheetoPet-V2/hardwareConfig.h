#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <Arduino.h>

// ---- PINS AND DEFINITIONS ----

/* ---- INFO ----
  i2c is the communication pretty much all the peripherals use.
  this includes the display, RTC (clock), gyro, ADC (voltage sensor for battery), and EEPROM (where save data is stored).
  SCL and SDA are the two pins it uses to communicate. they are shared across all devices. make sure these are set correctly below!
*/

// ---- I2C ----
constexpr int SDA_ALT = 9;              // default 20. alternate i2c SDA (i2c data) pin assignment
constexpr int SCL_ALT = 8;               // default 9. alternate i2c SCL/SCK (i2c clock) pin assignment

// ---- PERIPHERALS ----
constexpr int SPKR_PIN = 3;              // default 3. pin connected to speaker (must be pwm)
constexpr int MPU_ADDRESS = 0x69;        // default 0x69 (nice). if ADDR is pulled high, its 0x69, otherwise its 0x68. 0x68 will most likely conflict with DS3231 so dont make it that. lol
constexpr int SERIAL_BAUDRATE = 921600;  // default 921600. sets baudrate of serial. make sure serial monitor baudrate is the same.

// ---- BATTERY CHARGER ----
constexpr int CHRG_PIN = 4; // default 4. the CHRG pin of the TP4056 (battery charger).
constexpr int STDBY_PIN = 1; // default 1. the STDBY pin of the TP4056.

// ---- DISPLAY ----
constexpr int SCREEN_WIDTH = 128;   // display width (dont need to change unless you've got a goofy setup)
constexpr int SCREEN_HEIGHT = 128;  // display height
constexpr int DISPLAY_ADDRESS = 0x3C;    // default 0x3C. you wont need to change this most of the time
constexpr int OLED_RESET = -1;

// ---- EEPROM ----
constexpr int EEPROM_ADDRESS = 0x50;     // default 0x57, check your chip if unsure
constexpr int EEPROM_SIZE = 4096;        // default 4096. size in bytes. AT24C32 = 4KB

// ---- BUTTONS/SWITCHES ----
constexpr gpio_num_t SWITCH_PIN = GPIO_NUM_10;   // default 0. write as GPIO_NUM_{pin}
constexpr int B_BUTTON = 5;       // default 5. left button / B button pin
constexpr int X_BUTTON = 6;     // default 6. middle button / X button pin
constexpr int A_BUTTON = 7;      // default 7. right button / A button pin

// ---- MISC ----
constexpr int OFFSET = 4;   // screen recording offset, adjust until it lines up. prolly dont need to change this anyway

inline bool spkrEnable = true;

#endif
