# cheetoPet
<strong>an esp32 c3 based tamagotchi</strong>
<br><br>
# features
6 axis gyro <br>
128x128 monochrome oled display <br>
RTC module for timekeeping <br>
esp32 c3 super mini, low power but very capable <br>
# how to build
unfortunately i have not made a pcb for this yet, i built mine on a perfboard. heres the wiring:<br>
SDA = GPIO20, SCL = GPIO9 (modified since on board LED uses default sda pin) <br>
all i2c devices (rtc module, display, gyro) connect to power and i2c communication lines <br>
