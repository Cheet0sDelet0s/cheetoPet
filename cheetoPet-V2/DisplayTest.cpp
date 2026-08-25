#include "drivers/Platform.h"

#include "drivers/display/fonts/Font5x7.h"

using namespace display;

int main()
{
    platform::RTC clock;
    platform::Display display;
    platform::Input buttons;
    platform::Battery bat;

    if (!display.begin())
        return 1;
    
    if (!buttons.begin())
        return 1;

    if (!clock.begin())
        return 1;
    
    if (!bat.begin())
        return 1;

    display.fillScreen(BLACK); 

    display.setFont(fonts::Font5x7);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 30);
    display.println("the quick brown fox jumps over the lazy dog.");
    display.setCursor(0, 50);
    display.println("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG!");
    display.println("!?$%^&*():;@#/|-_~");

    display.present();

    while (display.processEvents())
    {
        buttons.update();

        static int x = 0;
        static int y = 0;

        if (buttons.isPressed(input::Button::Up)) y -= 1;
        if (buttons.isPressed(input::Button::Down)) y += 1;
        if (buttons.isPressed(input::Button::X)) x += 1;
        if (buttons.isPressed(input::Button::A)) x -= 1;
        
        display.drawPixel(x, y, RED);
        display.fillCircle(100, 100, 10, RED);

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