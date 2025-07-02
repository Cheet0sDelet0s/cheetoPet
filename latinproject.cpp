#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <latinproject.h>


const char* title1 = "Circe's warning";

const char* info1 = "Before setting sail again, the enchantress Circe warns Odysseus about the deadly allure of the Sirens, whose song lures sailors to their doom.";

const char* title2 = "A clever plan";

const char* info2 = "Following her advice, Odysseus orders his crew to plug their ears with beeswax so they won't hear the Sirens' song.";

const char* title3 = "Tempting fate";

const char* info3 = "Curious to hear the Sirens himself, Odysseus has his men tie him tightly to the mast of the ship and instructs them not to release him, no matter how much he begs.";

const char* title4 = "The haunting song";

const char* info4 = "As they sail past the Sirens' island, their voices rise-sweet, hypnotic, promising knowledge and beauty beyond compare. Odysseus is enchanted and struggles to break free.";

const char* title5 = "Safe passage";

const char* info5 = "True to their word, the crew holds firm and sails past unscathed. The Sirens' voices fade, and Odysseus survives the encounter wiser but deeply shaken.";

void latinProject() {
  const char* titles[5] = {title1, title2, title3, title4, title5};
  const char* infos[5] = {info1, info2, info3, info4, info5};

  waitForSelectRelease();

  while (!rightButtonState) {
    for (int i = 0; i < 5; i++) {
      display.clearDisplay();

      // Show title in big font
      display.setTextSize(2);  // Bigger text for title
      display.setTextColor(SH110X_WHITE);

      // Center title horizontally
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds(titles[i], 0, 0, &x1, &y1, &w, &h);
      int16_t x = (128 - w) / 2;  // Center horizontally
      int16_t y = (128 - h) / 2;  // Center vertically roughly

      display.setCursor(x, y);
      display.print(titles[i]);
      display.display();

      delay(3000);  // Wait 3 seconds on title

      // Show info in smaller font, fill screen
      display.clearDisplay();
      display.setTextSize(1.5);
      display.setCursor(0, 0);

      display.print(infos[i]);

      display.display();

      delay(5000);

      updateButtonStates();

      if (rightButtonState) {
        break;
      }
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print("The end");
    display.display();
    delay(3000);
  }
}