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
#include "pet.h"
#include "games.h"

void osStartup();
void handleMenuButtons();
void loadSavedGame();
void createNewGame();
void temporaryGame();
void beginOS();
void setCurrentMenu(String name);
bool addToList(int list[], int& itemCount, int maxSize, int value);
int indexOf(ItemList array[], int length, int targetType);
bool removeFromList(int list[], int& itemCount, int index);
int indexOfList(int array[], int length, int target);
bool isInArray(int item, int arr[]);
int promptDifficulty();
void handleSleepMode();

#endif