#include "pet.h"
#include "os.h"

bool readyToStart = false;

//unsigned long lastButtonTime = 0;
//const unsigned long debounceMs = 300;

MenuItem fileMenuItems[4] = {
  { "continue", [](){ loadSavedGame(); } },
  { "new game", [](){ createNewGame(); } },
  { "temporary game", [](){ temporaryGame(); } },
  { "go to sleep", [](){ lightSleep(1); } }
};

MenuItem mainMenuItems[3] = {
  { "pet", [](){ setCurrentMenu("pet"); } },
  { "games", [](){ setCurrentMenu("games"); } },
  { "settings", [](){ setCurrentMenu("settings"); } }
};

MenuItem petMenuItems[4] = {
  { "back", [](){ setCurrentMenu("pet"); } },
  { "inventory", [](){ setCurrentMenu("inventory"); } },
  { "edit", [](){ itemToPackUp = 0; setCurrentMenu("pet"); } },
  { "shop", [](){ setCurrentMenu("shop"); } }
};

MenuItem shopMenuItems[12] = {
  { "back", [](){ setCurrentMenu("pet menu"); }},
  { "couch - $5.00", [](){ purchaseItem(3, 5); } },
  { "stool - $2.50", [](){ purchaseItem(4, 2.50); } },
  { "fireplace - $7.50", [](){ purchaseItem(5, 7.50); } },
  { "table - $10.00", [](){ purchaseItem(6, 10); } },
  { "piano - $15.00", [](){ purchaseItem(37, 15); } },
  { "window - $12.50", [](){ purchaseItem(7, 12.50); } },
  { "rug - $4.00", [](){ purchaseItem(19, 4); } },
  { "tree - $15.00", [](){ purchaseItem(30, 15); } },
  { "bush - $7.50", [](){ purchaseItem(31, 7.50); } },
  { "grass - $1.00", [](){ purchaseItem(34, 1); } },
  { "grass 2 - $1.00", [](){ purchaseItem(35, 1); } }
};

MenuItem settingsItems[5] = {
  { "back", [](){ setCurrentMenu("main menu"); }},
  { "brightness", [](){ setCurrentMenu("brightness"); }},
  { "sound", [](){ setCurrentMenu("sound"); }},
  { "gyro", [](){ setCurrentMenu("gyro"); }},
  { "restart", [](){ esp_restart(); }},
};

MenuItem brightnessItems[5] = {
  { "1", [](){ display.setContrast(25); setCurrentMenu("settings"); }},
  { "2", [](){ display.setContrast(102); setCurrentMenu("settings"); }},
  { "3", [](){ display.setContrast(153); setCurrentMenu("settings"); }},
  { "4", [](){ display.setContrast(204); setCurrentMenu("settings"); }},
  { "5", [](){ display.setContrast(255); setCurrentMenu("settings"); }}
};

Menu fileMenu = {
  fileMenuItems,
  sizeof(fileMenuItems) / sizeof(fileMenuItems[0]),
  "file"
};

Menu mainMenu = {
  mainMenuItems,
  sizeof(mainMenuItems) / sizeof(mainMenuItems[0]),
  "main menu"
};

Menu petMenu = {
  petMenuItems,
  sizeof(petMenuItems) / sizeof(petMenuItems[0]),
  "pet menu"
};

Menu shopMenu = {
  shopMenuItems,
  sizeof(shopMenuItems) / sizeof(shopMenuItems[0]),
  "shop"
};

Menu settingsMenu = {
  settingsItems,
  sizeof(settingsItems) / sizeof(settingsItems[0]),
  "settings"
};

Menu brightnessMenu = {
  brightnessItems,
  sizeof(brightnessItems) / sizeof(brightnessItems[0]),
  "brightness"
};

String currentMenuName = "main menu";
Menu* currentMenu = &fileMenu;

bool addToList(int list[], int& itemCount, int maxSize, int value) {
  if (itemCount < maxSize) {
    list[itemCount++] = value;
    return true;
  } else {
    return false;
  }
}

void setCurrentMenu(String name) {
  currentMenuItem = 0;
  currentMenuName = name;

  if (name == "main menu") {
    currentMenu = &mainMenu;
  } else if (name == "pet menu") {
    currentMenu = &petMenu;
  } else if (name == "shop") {
    currentMenu = &shopMenu;
  } else if (name == "settings") {
    currentMenu = &settingsMenu;
  } else if (name == "brightness") {
    currentMenu = &brightnessMenu;
  }
}

void loadSavedGame() {
  showPopup("load save game", 1000);
  // loadGameFromEEPROM();
  readyToStart = true;
}

void createNewGame() {
  showPopup("create new game", 1000);
  readyToStart = true;
}

void temporaryGame() {
  showPopup("temporary game", 1000);
  readyToStart = true;
}

int indexOf(ItemList array[], int length, int targetType) {
  for (int i = 0; i < length; i++) {
    if (array[i].type == targetType) {
      return i;
    }
  }
  return -1; // Not found
}

int indexOfList(int array[], int length, int target) {
  for (int i = 0; i < length; i++) {
    if (array[i] == target) {
      return i;
    }
  }
  return -1;  // Not found
}

bool removeFromList(int list[], int& itemCount, int index) {
  if (index < 0 || index >= itemCount) {
    return false;  // Invalid index
  }

  // Shift elements left to fill the gap
  for (int i = index; i < itemCount - 1; i++) {
    list[i] = list[i + 1];
  }

  return true;  // Success
}

void handleMenuButtons() {
  if (buttonPressedThisFrame(1)) {
    currentMenuItem--;
  }

  if (buttonPressedThisFrame(2)) {
    currentMenuItem++;
  }

  if (buttonPressedThisFrame(3)) {
    if (currentMenuName == "inventory") {
      if (currentMenuItem == 0) {
        setCurrentMenu("pet menu");
      } else {
        placeItemFromInventory(inventory[currentMenuItem - 1]); // -1 because when drawing inventory, pushes all 
      }                                                         // items forward one space to allow "back" option
    } else {
      currentMenu->items[currentMenuItem].action();
    }
  }
}

void handleSleepMode() {
  if (powerSwitchState) {
    clearTones();
    //blindCloseAnimation();
    Serial.println("going into light sleep, see ya later!");
    display.clearDisplay();
    display.setCursor(12, 10);
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.print("goodnight");
    display.setTextSize(1);
    int randomiser = random(0, 4);
    String text;
    switch (randomiser) {
      case 0: text = "cya later human!"; break;
      case 1: text = "g'night human..."; break;
      case 2: text = "check on me later!"; break;
      case 3: text = "*yawn* cya later..."; break;
    }

    drawCenteredText(text.c_str(), 50);

    BitmapInfo petBmp = bitmaps[pets[userPet].stillID];
    int centeredX = (128 - petBmp.width) / 2;

    drawBitmapFromList(centeredX, 100, 1, pets[userPet].stillID, SH110X_WHITE);
    
    display.display();
    dumpBufferASCII();
    delay(1000);
    DateTime timeWhenSlept = rtc.now();
    
    lightSleep(0);
    
    DateTime now = rtc.now();

    TimeSpan timeSinceSlept = now - timeWhenSlept;

    int32_t seconds = timeSinceSlept.totalseconds();
    int32_t minutesSinceSlept = seconds / 60;

    petSleep += minutesSinceSlept;

    petPoop += constrain(minutesSinceSlept * 4, 0, 100);
    
    petHunger -= minutesSinceSlept / 30;
    
    petHunger = constrain(petHunger, 5, 999);

    petSleep = constrain(petSleep, 0, 120);

    //blindOpenAnimation();

    display.clearDisplay();
    display.setCursor(40, 10);
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.print("good");
    display.setCursor(25, 30);
    display.print("morning");
    display.setTextSize(1);
    display.setCursor(17, 70);
    display.print("my SLP is at ");
    display.print(petSleep);
    display.setCursor(17, 80);
    display.print("my HUN is at ");
    display.print(petHunger);

    petBmp = bitmaps[pets[userPet].stillID];
    centeredX = (128 - petBmp.width) / 2;

    drawBitmapFromList(centeredX, 100, 1, pets[userPet].stillID, SH110X_WHITE);
    
    display.display();
    dumpBufferASCII();

    // if (minutesSinceSlept >= saveInterval) {
    //   delay(700);
    //   saveGameToEEPROM(true);
    // } else {
    //   delay(1000);
    // }
  }
}

void osStartup() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  drawCenteredText("cheetoPet", 0);
  display.setTextSize(1);
  drawCenteredText("booting into OS V2", 20);
  drawCenteredText("by @Cheet0sDelet0s", 29);
  drawCenteredText("batt at " + String(getBatteryVoltage()) +"v", 50);
  drawBitmapFromList(55, 100, 1, 0, SH110X_WHITE);
  display.display();
  dumpBufferASCII();
  delay(500);
  

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    // set rtcs time to sketch compile time
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("rtc module lost power!\ntime & date has\nbeen reset.\nbooting in 3 secs");
    display.println("make sure the coin cell\ndidnt fall out\nor has lost charge!");
    display.display();
    delay(3000);
  }

  while (!readyToStart) {
    display.clearDisplay();
    handleMenuButtons();
    drawMenu(currentMenu, currentMenu->length, "game select");
    
    updatePreviousStates();
    display.display();
    delay(20);
  }
}

void beginOS() {
  updateButtonStates();
  setCurrentMenu("pet");
  while (1) {
    batteryManagement();
    
    updatePet();
    display.clearDisplay();

    if (currentMenuName == "pet") {
      handlePetButtons();
      drawPetHome();
    } else if (currentMenuName == "main menu" || currentMenuName == "pet menu" || currentMenuName == "shop" || currentMenuName == "settings" || currentMenuName == "brightness") {
      drawMenu(currentMenu, currentMenu->length, currentMenuName);
      handleMenuButtons();
    } else if (currentMenuName == "inventory") {
      drawInventory();
      handleMenuButtons();
    }

    updatePreviousStates();
    
    display.display();
    handleSleepMode();
    delay(5);
  }
}

// SAVING AND LOADING