// #include <ESP_I2S.h>
// #include <math.h>

// I2SClass I2S;

// // MAX98357A connections
// constexpr int PIN_BCLK  = 17;
// constexpr int PIN_LRCLK = 18;
// constexpr int PIN_DOUT  = 21;
// constexpr int PIN_SD    = 13;

// constexpr uint32_t SAMPLE_RATE = 44100;
// constexpr float TONE_FREQ = 440.0f;
// constexpr int16_t AMPLITUDE = 12000;


// #define I2C_SDA_PIN 8
// #define I2C_SCL_PIN 9

// #define A_BUTTON_PIN 0
// #define B_BUTTON_PIN 2
// #define X_BUTTON_PIN 46

// #define ROCK_L_PIN 41
// #define ROCK_R_PIN 42
// #define ROCK_SW_PIN 40

// void setup() {
//     Serial.begin(115200);
//     delay(500);
//     Serial.println("Hello world! Initing i2c");
//     delay(500);

//     pinMode(I2C_SDA_PIN, OUTPUT);
//     pinMode(I2C_SCL_PIN, OUTPUT);

//     pinMode(A_BUTTON_PIN, INPUT);
//     pinMode(B_BUTTON_PIN, INPUT);
//     pinMode(X_BUTTON_PIN, INPUT);
    
//     pinMode(ROCK_L_PIN, INPUT_PULLUP);
//     pinMode(ROCK_R_PIN, INPUT_PULLUP);
//     pinMode(ROCK_SW_PIN, INPUT_PULLUP);
    
//     digitalWrite(I2C_SDA_PIN, LOW);
//     digitalWrite(I2C_SCL_PIN, LOW);
//     // Wire.begin(8, 9);

//     pinMode(PIN_SD, OUTPUT);
//     digitalWrite(PIN_SD, HIGH);    // Enable amplifier

//     // BCLK, LRCLK, DOUT, DIN (unused), MCLK (unused)
//     I2S.setPins(PIN_BCLK, PIN_LRCLK, PIN_DOUT, -1, -1);

//     if (!I2S.begin(
//             I2S_MODE_STD,
//             SAMPLE_RATE,
//             I2S_DATA_BIT_WIDTH_16BIT,
//             I2S_SLOT_MODE_STEREO))
//     {
//         while (true) {
//         delay(100);
//         }
//     }
// }

// void showButtonStatus(int pin, String name, bool active) {
//     Serial.print(name);
//     Serial.print(" button state: ");
//     if (digitalRead(pin) == active) {
//         Serial.println("pressed");
//     } else {
//         Serial.println("not pressed");
//     }
// }

// void loop() {
//     // Wire.beginTransmission(0x36);
//     // Serial.println(Wire.endTransmission());

//     // Wire.beginTransmission(0x52);
//     // Serial.println(Wire.endTransmission());
//     Serial.println("running blah blah blah");

//     showButtonStatus(A_BUTTON_PIN, "A", false);
//     showButtonStatus(B_BUTTON_PIN, "B", false);
//     showButtonStatus(X_BUTTON_PIN, "X", true);
//     showButtonStatus(ROCK_L_PIN, "rocker left", false);
//     showButtonStatus(ROCK_R_PIN, "rocker right", false);
//     showButtonStatus(ROCK_SW_PIN, "rocker button", false);

//     Serial.println();
    
//     constexpr int SAMPLES = 256;
//     int16_t buffer[SAMPLES * 2];   // Stereo interleaved

//     static float phase = 0.0f;
//     const float phaseInc = 2.0f * PI * TONE_FREQ / SAMPLE_RATE;

//     for (int i = 0; i < SAMPLES; i++) {
//         int16_t sample = (int16_t)(AMPLITUDE * sinf(phase));

//         buffer[2 * i]     = sample;   // Left
//         buffer[2 * i + 1] = sample;   // Right

//         phase += phaseInc;
//         if (phase >= 2.0f * PI)
//         phase -= 2.0f * PI;
//     }

//     I2S.write((uint8_t *)buffer, sizeof(buffer));
// }

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:

  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = SPI2_HOST;   // ESP32-S3 FSPI
      cfg.spi_mode = 3;
      cfg.freq_write = 27000000;
      cfg.freq_read = 16000000;

      cfg.pin_sclk = 12;
      cfg.pin_mosi = 11;
      cfg.pin_miso = -1;
      cfg.pin_dc   = 7;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs   = 10;
      cfg.pin_rst  = 14;

      cfg.panel_width  = 240;
      cfg.panel_height = 280;

      cfg.offset_x = 0;
      cfg.offset_y = 0;

      cfg.invert = true;      // common for ST7789
      cfg.rgb_order = false; // try true if colours are swapped

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

LGFX tft;


void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("LovyanGFX start");

  tft.init();

  Serial.println("Display init OK");

  tft.setRotation(0);

  tft.fillScreen(TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);

  tft.drawString("ESP32-S3", 20, 20);
  tft.drawString("ST7789 240x280", 20, 50);

  tft.fillCircle(120, 140, 50, TFT_RED);
  tft.fillRect(70, 210, 100, 30, TFT_BLUE);
}

void loop()
{
}

// #include <SPI.h>

// // Pin definitions
// #define PIN_MISO 5
// #define PIN_MOSI 11
// #define PIN_SCK  12
// #define PIN_CS   6
// #define PIN_INT1 38
// #define PIN_INT2 39

// SPIClass spi(FSPI);

// // LSM6DSL registers
// #define WHO_AM_I      0x0F
// #define CTRL1_XL      0x10
// #define CTRL2_G       0x11
// #define CTRL3_C       0x12

// #define OUTX_L_G      0x22
// #define OUTX_L_XL     0x28

// // Expected device ID
// #define LSM6DSL_ID    0x6A

// void writeReg(uint8_t reg, uint8_t value) {
//   digitalWrite(PIN_CS, LOW);
//   spi.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE3));
//   spi.transfer(reg & 0x7F); // Write
//   spi.transfer(value);
//   spi.endTransaction();
//   digitalWrite(PIN_CS, HIGH);
// }

// uint8_t readReg(uint8_t reg) {
//   digitalWrite(PIN_CS, LOW);
//   spi.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE3));
//   spi.transfer(reg | 0x80); // Read
//   uint8_t value = spi.transfer(0);
//   spi.endTransaction();
//   digitalWrite(PIN_CS, HIGH);
//   return value;
// }

// void readRegs(uint8_t reg, uint8_t *buffer, uint8_t length) {
//   digitalWrite(PIN_CS, LOW);
//   spi.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE3));
//   spi.transfer(reg | 0xC0); // Read + auto-increment
//   for (int i = 0; i < length; i++) {
//     buffer[i] = spi.transfer(0);
//   }
//   spi.endTransaction();
//   digitalWrite(PIN_CS, HIGH);
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   pinMode(PIN_CS, OUTPUT);
//   digitalWrite(PIN_CS, HIGH);

//   pinMode(PIN_INT1, INPUT);
//   pinMode(PIN_INT2, INPUT);

//   spi.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

//   Serial.println("LSM6DSL SPI Test");

//   uint8_t id = readReg(WHO_AM_I);

//   Serial.print("WHO_AM_I = 0x");
//   Serial.println(id, HEX);

//   if (id != LSM6DSL_ID) {
//     Serial.println("ERROR: Device not found!");

//     for (int reg = 0x0F; reg <= 0x12; reg++) {
//       Serial.print("0x");
//       Serial.print(reg, HEX);
//       Serial.print(" = 0x");
//       Serial.println(readReg(reg), HEX);
//     }
//     while (1) delay(100);
//   }

//   Serial.println("Device detected.");

//   // Enable register auto increment (IF_INC = bit2)
//   writeReg(CTRL3_C, 0x04);

//   // Accelerometer:
//   // ODR = 104 Hz
//   // ±2 g
//   writeReg(CTRL1_XL, 0x40);

//   // Gyroscope:
//   // ODR = 104 Hz
//   // ±250 dps
//   writeReg(CTRL2_G, 0x40);

//   delay(100);

//   Serial.println("Sensors enabled.");
// }

// void loop() {
//   uint8_t data[12];

//   // Read gyro + accel
//   readRegs(OUTX_L_G, data, 12);

//   int16_t gx = (int16_t)(data[1] << 8 | data[0]);
//   int16_t gy = (int16_t)(data[3] << 8 | data[2]);
//   int16_t gz = (int16_t)(data[5] << 8 | data[4]);

//   int16_t ax = (int16_t)(data[7] << 8 | data[6]);
//   int16_t ay = (int16_t)(data[9] << 8 | data[8]);
//   int16_t az = (int16_t)(data[11] << 8 | data[10]);

//   Serial.print("Accel: ");
//   Serial.print(ax);
//   Serial.print(", ");
//   Serial.print(ay);
//   Serial.print(", ");
//   Serial.print(az);

//   Serial.print("   Gyro: ");
//   Serial.print(gx);
//   Serial.print(", ");
//   Serial.print(gy);
//   Serial.print(", ");
//   Serial.println(gz);

//   delay(100);
// }