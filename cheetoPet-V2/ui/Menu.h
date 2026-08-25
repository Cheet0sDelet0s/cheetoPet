#pragma once

#include "Button.h"

#include <cstddef>

namespace ui {

class Menu
{
public:
    static constexpr int MAX_ITEMS = 16;

    Menu(
        int x,
        int y,
        int width,
        int itemHeight = 32,
        int spacing = 4
    );

    bool addItem(
        const char* text,
        ButtonCallback callback = nullptr
    );

    void update(
        input::Input& input
    );

    void draw(
        display::Display& display
    );

    void clear();

    int itemCount() const;
    int selectedIndex() const;

    void setSelectedIndex(int index);

    Button* item(int index);
    const Button* item(int index) const;

private:
    void selectNext();
    void selectPrevious();

    int x_;
    int y_;
    int width_;

    int itemHeight_;
    int spacing_;

    Button items_[MAX_ITEMS];

    int itemCount_;
    int selectedIndex_;
};

} // namespace ui