#include "drivers/Platform.h"
#include "ui/Ui.h"
#include "ui/Menu.h"

#include "drivers/display/fonts/Font5x7.h"

#include <algorithm>

using namespace display;

void thing1() {printf("thing 1");}

void thing2() {printf("thing 2");}

void thing3() {printf("thing 3");}

const ui::Theme themey = {
    .background = display::Color(0x0000),
    .primary = display::Color(0xfb40),
    .focused = display::Color(0x05FF),
    .text = display::Color(0xFFFF),
    .disabled = display::Color(0x4208),
    .border = display::Color(0x6b59),
    .focusedBorder = display::Color(0xFFFF),
    .cornerRadius = 6
};

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
        200,    // width
        20,     // item height
        5       // spacing
    );

    ui::Menu settingsMenu(
        20,
        30,
        200,
        20,
        5
    );

    for (int i = 1; i < 5; i++)
    {
        mainMenu.addItem(
            "This is the main menu",
            thing1
        );

        mainMenu.addItem(
            "It is quite main",
            thing1
        );
    }

    for (int i = 1; i < 5; i++)
    {
        settingsMenu.addItem(
            "This is the settings menu",
            thing1
        );

        settingsMenu.addItem(
            "It is quite setty",
            thing1
        );
    }

    mainMenu.setTheme(themey);

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