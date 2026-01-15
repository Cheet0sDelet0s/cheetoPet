#include "os.h"

unsigned long lastButtonTime = 0;
const unsigned long debounceMs = 300;

MenuItem fileMenuItems[4] = {
  { "continue", [](){ loadSavedGame(); } },
  { "new game", [](){ createNewGame(); } },
  { "temporary game", [](){ temporaryGame(); } },
  { "go to sleep", [](){ lightSleep(1); } }
};

Menu fileMenu = {
  fileMenuItems,
  sizeof(fileMenuItems) / sizeof(fileMenuItems[0]),
  "File"
};

String currentMenuName = "main menu";
Menu* currentMenu = &fileMenu;

void loadSavedGame() {
  showPopup("load save game", 1000);
  // loadGameFromEEPROM();
  beginOS();
}

void createNewGame() {
  showPopup("create new game", 1000);
  beginOS();
}

void temporaryGame() {
  showPopup("temporary game", 1000);
  beginOS();
}

void handleMenuButtons() {
  if (millis() - lastButtonTime < debounceMs) return;
  updateButtonStates();

  if (leftButtonState) {
    currentMenuItem--;
    lastButtonTime = millis();
  }

  if (middleButtonState) {
    currentMenuItem++;
    lastButtonTime = millis();
  }

  if (rightButtonState) {
    currentMenu->items[currentMenuItem].action();
    lastButtonTime = millis();
  }
}

void osStartup() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  drawCenteredText("cheetoPet V2", 0);
  display.setTextSize(1);
  drawCenteredText("booting into OS...", 20);
  drawCenteredText("by @Cheet0sDelet0s", 29);
  drawCenteredText("batt at " + String(getBatteryVoltage()) +"v", 50);
  drawBitmapFromList(55, 100, 1, 0, SH110X_WHITE);
  display.display();
  dumpBufferASCII();
  delay(500);
  

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    // Set the RTC to the current date & time
    rtc.adjust(DateTime(2025, 6, 6, 7, 53, 0));
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("rtc module lost power!\ntime & date has\nbeen reset.\nbooting in 3 secs");
    display.println("make sure the coin cell\ndidnt fall out\nor has lost charge!");
    display.display();
    delay(3000);
  }

  bool readyToStart = false;
  while (!readyToStart) {
    display.clearDisplay();
    handleMenuButtons();
    drawMenu(currentMenu, currentMenu->length, "game select");
    
    display.display();
    delay(20);
  }
}

void beginOS() {
  while (1) {
    updatePet();
    display.clearDisplay();
    drawPetHome();
    display.display();
    delay(5);
  }
}

// SAVING AND LOADING