/* ---- os.h ----

  holds all functions for general operating system stuff, like startup splash, general menus, etc

*/

#ifndef OS_H
#define OS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <array>

// ---- CHEETOPET FILES ----
#include "hardware.h"
#include "graphics.h"
#include "bitmaps.h"

void osStartup();
void handleMenuButtons();
void loadSavedGame();
void createNewGame();
void temporaryGame();

#endif