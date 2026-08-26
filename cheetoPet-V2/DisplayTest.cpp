#include "drivers/Platform.h"
#include "ui/Ui.h"
#include "ui/Menu.h"

#include "drivers/display/fonts/Font5x7.h"

#include <algorithm>

using namespace display;

void thing1() {printf("thing 1");}

void thing2() {printf("thing 2");}

void thing3() {printf("thing 3");}

int main()
{
    platform::RTC clock;
    platform::Display display;
    platform::Input buttons;
    platform::Battery bat;
    platform::IMU imu;
    platform::Audio audio;

    if (!display.begin())
        return 1;
    
    if (!buttons.begin())
        return 1;

    if (!clock.begin())
        return 1;
    
    if (!bat.begin())
        return 1;

    if (!imu.begin())
        return 1;
    
    if (!audio.begin())
        return 1;

    audio.setVolume(0.5f);
    audio.tone(440,200);
    
    imu.setMouseControlEnabled(true);

    display.fillScreen(BLACK); 

    display.setFont(fonts::Font5x7);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    
    ui::Menu mainMenu(
        20,     // x
        30,     // y
        100,    // width
        15,     // item height
        5       // spacing
    );

    mainMenu.addItem(
        "Thing 1",
        thing1
    );

    mainMenu.addItem(
        "Thing 2",
        thing2
    );

    mainMenu.addItem(
        "Thing 3",
        thing3
    );

    ui::Ui ui(
        display,
        buttons
    );

    ui.setMenu(&mainMenu);

    display.present();

    while (display.processEvents())
    {
        buttons.update();
        imu.update();
        ui.update();

        auto gyro = imu.gyro();
        auto accel = imu.acceleration();

        display.fillScreen(BLACK);

        ui.draw();

        // display.fillRoundRect(100, 200, 80, 40, 5, GREEN);
        // display.drawPixel(100, 200, RED);
        // display.drawPixel(180, 240, RED);

        printf(
            "gyro: %.2f %.2f %.2f\n",
            gyro.x,
            gyro.y,
            gyro.z
        );

        printf(
            "accel: %.2f %.2f %.2f\n",
            accel.x,
            accel.y,
            accel.z
        );

        static int etchX = 0;
        static int etchY = 0;

        static int circleX = 64;
        static int circleY = 64;

        static int frequency = 200;

        if (buttons.isPressed(input::Button::Up)) etchY -= 1;
        if (buttons.isPressed(input::Button::Down)) etchY += 1;
        if (buttons.isPressed(input::Button::X)) etchX += 1;
        if (buttons.isPressed(input::Button::A)) etchX -= 1;

        if (buttons.wasPressed(input::Button::B))
        {
            frequency += 30;
            if (frequency > 2000) frequency = 200;
            audio.tone(frequency, 200);
        }

        circleX = std::clamp(int(circleX + accel.z / 2), 0, 240);
        circleY = std::clamp(int(circleY + accel.x / 2), 0, 280);
        
        display.drawPixel(etchX, etchY, RED);
        display.fillCircle(circleX, circleY, 10, GREEN);

        rtc::DateTime now;

        if (clock.read(now)) {
            display.fillRect(0, 0, 175, 16, BLACK);
            display.setTextColor(WHITE);
            display.setCursor(0, 0);
            display.print("time: ");
            display.print(now.hour);
            display.print(":");
            display.print(now.minute);
            display.print(":");
            display.print(now.second);
        }

        bat.read();

        display.print("  |  battery: ");
        display.print(bat.getPercentage() / 100);
        display.print("%");

        display.present();
    }

    return 0;
}