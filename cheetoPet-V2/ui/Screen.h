#pragma once

#include "../drivers/display/Display.h"
#include "../drivers/input/Input.h"
#include "Theme.h"

namespace ui {

class Screen
{
public:
    virtual ~Screen() = default;

    virtual void update(
        input::Input& input
    ) = 0;

    virtual void draw(
        display::Display& display
    ) = 0;

    virtual void setTheme(
        const Theme& theme
    ) = 0;

    virtual void onEnter() {}
    virtual void onExit() {}
};

} // namespace ui