#include "hardware.h"

#define MAX_STRING_LENGTH 128  // Maximum string length to read/write

// create display object (ignore any vscode errors)
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);
Adafruit_ADS1115 ads;  // Create ADS1115 object
RTC_DS3231 rtc;
DateTime tempDateTime;
MPU9250_asukiaaa mpu(MPU_ADDRESS);
AT24C04 eepromChip(EEPROM_ADDRESS);

// ---- buttons ----
DRAM_ATTR bool leftButtonState = false;
DRAM_ATTR bool middleButtonState = false;
DRAM_ATTR bool rightButtonState = false;
DRAM_ATTR bool powerSwitchState = false;
DRAM_ATTR bool previousLeftState = false;
DRAM_ATTR bool previousMiddleState = false;
DRAM_ATTR bool previousRightState = false;



// Tone queue
#define MAX_TONES 100
DRAM_ATTR Note noteQueue[MAX_TONES];
int toneCount = 0;  // number of tones currently in the queue

// Playback state
unsigned long lastStepTime = 0;
int currentTone = 0;
bool isPlaying = false;
DRAM_ATTR int playTime = 50;

/* ----------SONGS----------

write your own songs and put them here and in the songs[] array!
the syntax for each note is: { note frequency (hz) , note duration (ms) }

*/

Note odeToJoy[15] = {
  {329.63, 500}, {329.63, 500}, {349.23, 500}, {392.00, 500}, {392.00, 500}, {349.23, 500}, {329.63, 500},
  {293.66, 500}, {261.63, 500}, {261.63, 500}, {293.66, 500}, {329.63, 500}, {329.63, 750}, {293.66, 500},
  {293.66, 1000}
};

Note maryLamb[14] = { //written by AI
  {329.63, 500}, {293.66, 500}, {261.63, 500}, {293.66, 500}, {329.63, 500}, {329.63, 500}, {329.63, 1000},
  {293.66, 500}, {293.66, 500}, {293.66, 1000}, {329.63, 500}, {392.00, 500}, {392.00, 1000}, {329.63, 1000}
};

Note happyBirthday[25] = { //written by AI
  {264.00, 250}, {264.00, 250}, {297.00, 500}, {264.00, 500}, {352.00, 500}, {330.00, 1000}, 
  {264.00, 250}, {264.00, 250}, {297.00, 500}, {264.00, 500}, {396.00, 500}, {352.00, 1000},
  {264.00, 250}, {264.00, 250}, {528.00, 500}, {440.00, 500}, {352.00, 500}, {330.00, 500}, {297.00, 1000},
  {466.00, 250}, {466.00, 250}, {440.00, 500}, {352.00, 500}, {396.00, 500}, {352.00, 1000}
};

Song songs[] = {
  {odeToJoy, sizeof(odeToJoy) / sizeof(odeToJoy[0])},
  {maryLamb, sizeof(maryLamb) / sizeof(maryLamb[0])},
  {happyBirthday, sizeof(happyBirthday) / sizeof(happyBirthday[0])}
};

int numSongs = sizeof(songs) / sizeof(songs[0]);

// ---- screen recording ----
DRAM_ATTR unsigned long lastDump = 0;  // keeps track of the last time we dumped hahahah get it
bool screenRecording = false;

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
  display.setContrast(25);
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

void lightSleep(int type) {
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

  if (type == 1) {
    esp_restart();
  }
}

// ---- BATTERY ----

float getBatteryVoltage() {
  int16_t adc0 = ads.readADC_SingleEnded(0); // Read channel 0

  // Convert to volts (ADS1115: 0.125 mV per bit at default gain)
  float rawVoltage = adc0 * 0.125 / 1000.0;

  // Compensate for divider (2x)
  float batteryVoltage = rawVoltage * 2.0;

  currentBatteryVoltage = batteryVoltage;

  return batteryVoltage; // 3 decimal places
}

int getBatteryPercentage() {
  float batteryVoltage = getBatteryVoltage();
  
  // Clamp voltage to LiPo range
  if (batteryVoltage > 4.2) batteryVoltage = 4.2;
  if (batteryVoltage < 3.0) batteryVoltage = 3.0;

  // Map voltage to percentage (linear between 3.0V = 0% and 4.2V = 100%)
  int percentage = (int)((batteryVoltage - 3.0) / (4.2 - 3.0) * 100.0);
  currentBatteryPercentage = percentage;
  return percentage;
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

    lightSleep(0);
  }
}

void eepromWriteByte(uint16_t addr, uint8_t data) {
  eepromChip.put(addr, data);
}

uint8_t eepromReadByte(uint16_t addr) {
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

void bootMenu() {
  Serial.println("showing boot option menu");
  display.clearDisplay();
  drawCenteredText("A to boot", 60); // when powering on cheetoPet after power loss or reset, allow the user to choose whether to boot to OS
  display.display();
  bool shouldBoot = false;
  for (int i = 1; i < 255; i++) {
    updateButtonStates();
    if (rightButtonState) {
      Serial.println("user pressed A, booting!");
      shouldBoot = true;
      break;
    }
    delay(20);
  }

  if (!shouldBoot) { // go to sleep since user did not press A to boot. wake up when switch changes state
    lightSleep(0);
  }
}

void dumpBufferASCII() {
  uint8_t *buffer = display.getBuffer();
  const int w = display.width();
  const int h = display.height();
  const int rowsPerChunk = 8;  // how many rows to print at once
  char chunk[w * rowsPerChunk + rowsPerChunk + 1]; // +1 for final null, +rowsPerChunk for line breaks

  for (int y = 0; y < h; y += rowsPerChunk) {
    int chunkIndex = 0;

    for (int row = 0; row < rowsPerChunk && (y + row) < h; row++) {
      for (int x = 0; x < w; x++) {
        int byteIndex = x + ((y + row) / 8) * w;
        int bitMask = 1 << ((y + row) & 7);
        chunk[chunkIndex++] = (buffer[byteIndex] & bitMask) ? '#' : '.';
      }
      chunk[chunkIndex++] = '\n'; // add newline at end of row
    }

    chunk[chunkIndex] = '\0';  // null-terminate the chunk
    Serial.print(chunk);        // print multiple rows at once
  }

  Serial.println(); // final empty line to signal end of frame
}

void screenRecord() {
  if (millis() - lastDump >= 100 && screenRecording) {
      lastDump = millis();
      dumpBufferASCII();
  }
}

void waitForSelectRelease() {
  const unsigned long debounceDelay = 50;  // milliseconds
  unsigned long lastStableTime = millis();

  // Wait until the button is released and stable
  while (true) {
    if (digitalRead(A_BUTTON) == HIGH) {
      // Button is released, check how long it's been stable
      if (millis() - lastStableTime >= debounceDelay) {
        break;  // Released and stable
      }
    } else {
      // Button pressed again — reset the timer
      lastStableTime = millis();
    }
  }
  updateButtonStates();
}

void updateGyro() {
  mpu.gyroUpdate();
  mpu.accelUpdate();

  int gyroX = round((mpu.gyroY() + gyroYOffset) / 2) * 2 * -1;  //x value is inverted since gyro is upside down in hardware NOT IN PCB!!
  int gyroY = round((mpu.gyroX() + gyroXOffset) / 2) * 2 * -1;
  int gyroZ = round((mpu.gyroZ() + gyroZOffset) / 2) * 2;

  int accelX = round(mpu.accelY() + accelXOffset);
  int accelY = round(mpu.accelX() + accelYOffset);
  int accelZ = round(mpu.accelZ() + accelZOffset);

  // Serial.printf("X: %f, Y: %f, Z: %f\n", accelX, accelY, accelZ);

  totalG = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  unsigned long updateNow = millis();
  float deltaTime = (updateNow - lastUpdate) / 1000.0;
  lastUpdate = updateNow;

  angleX += gyroX * deltaTime;
  angleY += gyroY * deltaTime;
  angleZ += gyroZ * deltaTime;

  posX += gyroX * deltaTime;
  posY += gyroY * deltaTime;
  posZ += gyroZ * deltaTime;
}

void clearTones() {
  toneCount = 0;
  ledcWrite(SPKR_PIN, 0);
}

void queueTone(float freq, uint16_t length) {
  if (toneCount < MAX_TONES) {
    noteQueue[toneCount] = { freq, length };
    toneCount++;
  }
}

void priorityQueueTone(float freq, int length) {
  if (toneCount < MAX_TONES) {
    // Shift elements to the right
    for (int i = MAX_TONES - 1; i > 0; i--) {
      noteQueue[i] = noteQueue[i - 1];
    }

    Note newNote = {freq, length};

    // Insert at the start
    noteQueue[0] = newNote;
    toneCount++;
  }
}

void audioEngine() {
  if (toneCount > 0 && spkrEnable) {
    if (!isPlaying) {
      // Start playing first note
      currentTone = noteQueue[0].freq;
      playTime = noteQueue[0].length;
      ledcWriteTone(SPKR_PIN, currentTone);

      //Serial.print("Tone: "); Serial.print(currentTone);
      //Serial.print(" Length: "); Serial.println(playTime);

      lastStepTime = millis();
      isPlaying = true;
    } else if (millis() - lastStepTime >= playTime) {
      // Shift queue down by 1
      for (int i = 1; i < toneCount; i++) {
        noteQueue[i - 1] = noteQueue[i];
      }
      toneCount--;

      if (toneCount > 0) {
        // Play next note
        currentTone = noteQueue[0].freq;
        playTime = noteQueue[0].length;
        ledcWriteTone(SPKR_PIN, currentTone);

        //Serial.print("Tone: "); Serial.print(currentTone);
        //Serial.print(" Length: "); Serial.println(playTime);

        lastStepTime = millis();
      } else {
        // Finished all notes
        ledcWriteTone(SPKR_PIN, 0);
        isPlaying = false;
      }
    }
  } else if (!spkrEnable) {
    ledcWrite(SPKR_PIN, 0);
  }
}

void playRandomSong() {
  clearTones();

  int songIndex = random(numSongs); // Pick random song
  Song chosen = songs[songIndex];

  playSong(chosen);
}

void playSong(Song song) {
  for (int i = 0; i < song.length; i++) {
    queueTone(song.notes[i].freq, song.notes[i].length);
  }
}

bool buttonPressedThisFrame(int num) { // num is what button was pressed
  updateButtonStates();
  bool* button;
  bool* prevState;
  switch (num) {
    case 1:
      button = &leftButtonState;
      prevState = &previousLeftState; break;
    case 2:
      prevState = &previousMiddleState;
      button = &middleButtonState; break;
    case 3:
      prevState = &previousRightState;
      button = &rightButtonState; break;
  }
  bool pressed = false;
  if (*button == true && *button != *prevState) {
    pressed = true;
  }

  if (pressed) return true;
  return false;
}

void updatePreviousStates() {
  updateButtonStates();
  previousLeftState = leftButtonState;
  previousMiddleState = middleButtonState;
  previousRightState = rightButtonState;
}
void calibrateGyro() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(SH110X_WHITE);
  display.println("press A, then,");
  display.println("lay device flat on a surface.");
  display.println("wait 3 seconds, and the gyro");
  display.println("will start calibrating.");
  display.println("do not touch while calibrating");
  display.display();

  updateButtonStates();
  
  while (rightButtonState) {
    updateButtonStates();
    delay(10);
  }

  while (!rightButtonState) {
    updateButtonStates();
    delay(10);
  }

  waitForSelectRelease();

  updateButtonStates();

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("calibrating in 3 seconds...");
  display.display();

  delay(3000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("calibrating...");
  display.print("do not touch!!!");
  display.display();

  const int samples = 50;
  
  float xValues;
  float yValues;
  float zValues;

  for (int count = 0; count < samples; count++) {
    mpu.gyroUpdate();
    xValues += mpu.gyroX();
    yValues += mpu.gyroY();
    zValues += mpu.gyroZ();
    delay(5);
  }

  gyroXOffset = xValues / samples * -1; // get mean drift of each axis, apply offset in opposite direction! so simple! and its terrible!
  gyroYOffset = yValues / samples * -1;
  gyroZOffset = zValues / samples * -1;

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("done! you can pick me up");
  display.display();

  delay(3000);
}