#include "hardware.h"

#define MAX_STRING_LENGTH 128  // Maximum string length to read/write

// create display object (ignore any vscode errors)
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);
Adafruit_ADS1115 ads;  // Create ADS1115 object
RTC_DS3231 rtc;
DateTime tempDateTime;
MPU9250_asukiaaa mpu(MPU_ADDRESS);
AT24C04 eepromChip(EEPROM_ADDRESS);

DRAM_ATTR bool leftButtonState = false;
DRAM_ATTR bool middleButtonState = false;
DRAM_ATTR bool rightButtonState = false;
DRAM_ATTR bool powerSwitchState = false;
DRAM_ATTR bool previousLeftState = false;
DRAM_ATTR bool previousMiddleState = false;
DRAM_ATTR bool previousRightState = false;

void updateButtonStates() {
  leftButtonState = digitalRead(B_BUTTON) == LOW;
  middleButtonState = digitalRead(X_BUTTON) == LOW;
  rightButtonState = digitalRead(A_BUTTON) == LOW;
  powerSwitchState = digitalRead(SWITCH_PIN) == LOW;
}

void initDisplay() {
  display.begin(DISPLAY_ADDRESS, true);
  display.clearDisplay();
  display.display();
  display.setTextColor(SH110X_WHITE);
}

void initPins() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(CHRG_PIN, INPUT_PULLUP);
  pinMode(STDBY_PIN, INPUT_PULLUP);
  pinMode(A_BUTTON, INPUT_PULLUP);
  pinMode(B_BUTTON, INPUT_PULLUP);
  pinMode(X_BUTTON, INPUT_PULLUP);
  pinMode(SPKR_PIN, OUTPUT);

  ledcAttach(SPKR_PIN, 5000, 8);
  ledcWriteTone(SPKR_PIN, 0);     // start silent
}

void initPeripherals() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("begun serial at 921600 baudrate! hello world!");
  analogReadResolution(12);

  Wire.begin(SDA_ALT, SCL_ALT);

  Serial.println("begun i2c! beginning to initialise peripherals...");

  if (!ads.begin()) {
    Serial.println("failed to find ADS1115 chip, program halting!");
    while (1);
  }

  Serial.println("ads1115 found!");

  ads.setGain(GAIN_ONE); // ±4.096V range, 1 bit = 0.125mV

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    display.clearDisplay();
    display.println("bro i cant find the rtc module you are screwed");
    display.display();
    while (1) delay(10);
  }

  mpu.setWire(&Wire); // setup mpu9250 / mpu6050
  mpu.beginGyro();
  mpu.beginAccel();
}

void mpu9250_sleep() {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x6B);              // PWR_MGMT_1 register
  Wire.write(0x40);              // Set SLEEP bit (bit 6)
  Wire.endTransmission();
}

void mpu9250_wake() {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x6B);
  Wire.write(0x01);  // Use PLL with X-axis gyroscope as clock source
  Wire.endTransmission();
}

void prepareForSleepyTime() {
  display.clearDisplay();
  display.display();
  mpu9250_sleep();
}

void lightSleep() {
  prepareForSleepyTime();

  esp_sleep_enable_gpio_wakeup();
  updateButtonStates();

  if (powerSwitchState) { // set wakeup to opposite of current switch state
    gpio_wakeup_enable(SWITCH_PIN, GPIO_INTR_HIGH_LEVEL); // Wake up on switch (HIGH)
  } else {
    gpio_wakeup_enable(SWITCH_PIN, GPIO_INTR_LOW_LEVEL); // Wake up on switch (HIGH)
  }
  
  esp_light_sleep_start();  // sleepy time! program will resume after wake up
  mpu9250_wake();
}

// ---- BATTERY ----

float getBatteryVoltage() {
  int16_t adc0 = ads.readADC_SingleEnded(0); // Read channel 0

  // Convert to volts (ADS1115: 0.125 mV per bit at default gain)
  float rawVoltage = adc0 * 0.125 / 1000.0;

  // Compensate for divider (2x)
  float batteryVoltage = rawVoltage * 2.0;

  return batteryVoltage; // 3 decimal places
}

int getBatteryPercentage() {
  float batteryVoltage = getBatteryVoltage();
  
  // Clamp voltage to LiPo range
  if (batteryVoltage > 4.2) batteryVoltage = 4.2;
  if (batteryVoltage < 3.0) batteryVoltage = 3.0;

  // Map voltage to percentage (linear between 3.0V = 0% and 4.2V = 100%)
  return (int)((batteryVoltage - 3.0) / (4.2 - 3.0) * 100.0);
}

/* STATUSES:  (CHRG + STDBY)
0: battery discharging or no battey present (HIGH + HIGH)
1: battery charging (LOW + HIGH)
2: battery reached end of charging (HIGH + LOW)
3: holy shit idk wtf has happened but you are fucked (LOW + LOW)
*/
int getBatteryStatus() {
  bool chrgStatus = digitalRead(CHRG_PIN) == LOW;
  bool stdbyStatus = digitalRead(STDBY_PIN) == LOW;

  if (!chrgStatus && !stdbyStatus) { // HIGH + HIGH (discharging)
    return 0;
  } else if (chrgStatus && !stdbyStatus) { // LOW + HIGH (charging)
    return 1;
  } else if (!chrgStatus && stdbyStatus) { // HIGH + LOW (finished charging)
    return 2;
  } else if (chrgStatus && stdbyStatus) { // LOW + LOW (error, should never happen unless short or tp4056 damaged)
    return 3;
  }
}

void batteryManagement() {
  float bv = getBatteryVoltage();

  if (bv < 3.4) {
    Serial.println("battery voltage is too low at " + String(bv) + "v, going to sleep!!");
    display.setContrast(25);
    display.clearDisplay();
    drawBitmapFromList(58, 62, 1, 63, SH110X_WHITE);
    display.display();
    delay(2000);

    lightSleep();
  }
}

void eepromWriteByte(uint16_t addr, uint8_t data) {
  eepromChip.put(addr, data);
  // Wire.beginTransmission(EEPROM_ADDRESS);
  // Wire.write((addr >> 8) & 0xFF); // MSB
  // Wire.write(addr & 0xFF);        // LSB
  // Wire.write(data);
  // Wire.endTransmission();
  // delay(5); // EEPROM write cycle
}

uint8_t eepromReadByte(uint16_t addr) {
  // Wire.beginTransmission(EEPROM_ADDRESS);
  // Wire.write((addr >> 8) & 0xFF);
  // Wire.write(addr & 0xFF);
  // Wire.endTransmission();

  // Wire.requestFrom(EEPROM_ADDRESS, 1);
  // if (Wire.available()) {
  //   return Wire.read();
  // }
  // return 0xFF; // Default on fail
  int readData;
  eepromChip.get(addr, readData);
  return readData;
}

// Write a C-style string to EEPROM
void eepromWriteString(uint16_t addr, const char *str) {  // this should never of been made in the first place but im too lazy anyway
  uint16_t i = 0;
  while (str[i] != '\0' && i < MAX_STRING_LENGTH - 1) {
    eepromWriteByte(addr + i, str[i]);
    i++;
  }
  eepromWriteByte(addr + i, 0); // Null terminator
}

// Read a C-style string from EEPROM into a buffer
// The buffer must have at least MAX_STRING_LENGTH bytes
void eepromReadString(uint16_t addr, char *buffer, uint16_t bufferSize) {
  uint16_t i = 0;
  uint8_t c;

  while (i < bufferSize - 1 && i < MAX_STRING_LENGTH - 1) {
    c = eepromReadByte(addr + i);
    if (c == 0 || c == 0xFF) {
      break; // Stop at null terminator or invalid read
    }
    buffer[i] = (char)c;
    i++;
  }
  buffer[i] = '\0'; // Ensure string is null-terminated
}


uint16_t saveVectorToEEPROM(uint16_t addr, const std::vector<ItemList> &vec) {
  eepromWriteByte(addr++, vec.size()); // Save size
  for (const auto &item : vec) {
    eepromWriteByte(addr++, item.type);
    eepromWriteByte(addr++, item.x);
    eepromWriteByte(addr++, item.y);
  }
  return addr;
}

uint16_t loadVectorFromEEPROM(uint16_t addr, std::vector<ItemList> &vec) {
  vec.clear();
  uint8_t size = eepromReadByte(addr++);
  for (int i = 0; i < size; i++) {
    ItemList item;
    item.type = eepromReadByte(addr++);
    item.x = eepromReadByte(addr++);
    item.y = eepromReadByte(addr++);
    vec.push_back(item);
  }
  return addr;
}