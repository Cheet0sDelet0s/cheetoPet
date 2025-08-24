# cheetoPet
<strong>an esp32 c3 based tamagotchi</strong>
<br><br>

![main image](photos/main.jpg)
![side view](photos/side.jpg)
![screenshot1](photos/screenshot1.png)

<br>
post on my website for more info: https://www.cloudables.net/2025/08/22/cheetopet/
<br>

# features>
6 axis gyro <br>
128x128 monochrome oled display <br>
RTC module for timekeeping <br>
esp32 c3 super mini, low power but very capable <br>
# how to build
components: <br>
esp32 c3 super mini <br>
SH1107 128x128 oled display <br>
DS3231 rtc module <br>
MPU9250 gyro <br>
TP4056 charging board <br>
1s lipo battery (1000mah will last a week or so without charging, go higher if you want) <br>
3x smd push buttons <br>
2 position 3 pin switch <br>

PCB IS NOT FINISHED - DO NOT ORDER YET!

i built mine on a perfboard. heres the wiring:<br>
SDA = GPIO20, SCL = GPIO9 (modified since on-board rgb led uses default sda pin) <br>
all i2c devices (rtc module, display, gyro) connect to power and i2c communication lines <br>
left button => GPIO5 <br>
middle button => GPIO6 <br>
right button => GPIO7 <br>

power switch does not physically disconnect the battery, it just puts the chip in sleep mode.<br>
this allows faster wakeups and the entire program being saved without actually writing any data. (important data is still saved to eeprom chip)<br>
switch => GPIO0 <br>

i used a TP4056 charging board with the lipo, super easy to use: <br>
battery+ => TP4056 B+     battery- => TP4056 B- <br>
TP4056 OUT+ => ESP32C3 5V     TP4056 OUT- => ESP32C3 GND <br>

make sure to put a cr2032 in the coin cell slot of the RTC module to keep track of time and save your data! <br>

for the gyro module, make sure to pull the ADD pin high (connect it to 5v/vcc of the module). this sets the gyros i2c address to 0x69 (nice) instead of 0x68, which would clash with the rtcs' i2c address. if you do not do this it will not work!
