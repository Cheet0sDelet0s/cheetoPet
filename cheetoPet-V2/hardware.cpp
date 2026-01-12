#include "hardware.h"

// create display object (ignore any vscode errors)
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);
Adafruit_ADS1115 ads;  // Create ADS1115 object
RTC_DS3231 rtc;
DateTime tempDateTime;
MPU9250_asukiaaa mpu(MPU_ADDRESS);

void initDisplay() {
  display.begin(DISPLAY_ADDRESS, true);
  display.clearDisplay();
  display.display();
  display.setTextColor(SH110X_WHITE);
}