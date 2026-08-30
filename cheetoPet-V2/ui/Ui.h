#pragma once

#include "Screen.h"

namespace ui {

class Ui
{
public:
    static constexpr int MAX_SCREENS = 8;

    Ui(
        display::Display& display,
        input::Input& input
    );

    void update();
    void draw();

    bool push(Screen* screen);
    bool pop();

    void clear();

    Screen* current();
    const Screen* current() const;

    int screenCount() const;

    void setTheme(const Theme& theme);

    const Theme& theme() const;

private:
    display::Display& display_;
    input::Input& input_;

    Screen* screens_[MAX_SCREENS];

    int screenCount_;

    const Theme* theme_;
};

} // namespace ui