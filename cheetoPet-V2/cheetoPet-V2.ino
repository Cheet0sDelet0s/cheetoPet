 /***********************************\
 | cheetoPet V2 - by olly jeffery :) |
 \***********************************/

// ---- LIBRARIES ----
#include <Arduino.h>

// ---- CHEETOPET FILES ----
#include "hardware.h"
#include "os.h"

void setup() {
  initPins(); // set pinModes
  initPeripherals(); // begin serial, i2c, etc
  initDisplay(); // initialise display, clear it, set text color to white (i always forget to do this).
  batteryManagement();
  
  bootMenu(); // allow user to decide whether they want to boot into os
  // esp will sleep here if user doesnt decide to boot, will resume when power switch is toggled
  osStartup();

  beginOS(); // basically loop() equivalent
}

void loop() {
  // code will never reach loop unless something has gone terribly wrong. delay and debug here just in case
  Serial.println("CODE HAS REACHED LOOP() SOMEHOW. PROGRAM WILL DO NOTHING NOW :DD");
  display.clearDisplay();
  display.print("big boi error. code has reached loop()");
  display.display();
  delay(500);
}