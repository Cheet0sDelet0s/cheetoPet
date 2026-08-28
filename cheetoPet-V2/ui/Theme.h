#pragma once

#include "../drivers/display/Display.h"

namespace ui {

struct Theme
{
    display::Color background;

    display::Color primary;
    display::Color focused;
    display::Color text;
    display::Color disabled;

    display::Color border;
    display::Color focusedBorder;

    uint8_t cornerRadius;
};

}