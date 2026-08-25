#pragma once

#include "Menu.h"

namespace ui {

class Ui
{
public:
    Ui(
        display::Display& display,
        input::Input& input
    );

    void update();
    void draw();

    void setMenu(Menu* menu);

    Menu* menu();
    const Menu* menu() const;

private:
    display::Display& display_;
    input::Input& input_;

    Menu* menu_;
};

} // namespace ui