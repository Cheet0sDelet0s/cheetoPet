#pragma once

#include "Screen.h"
#include "Button.h"

namespace ui {

class Menu : public Screen
{
public:
    static constexpr int MAX_ITEMS = 16;

    Menu(
        int x,
        int y,
        int width,
        int height,
        int itemHeight = 32,
        int spacing = 4
    );

    bool addItem(
        const char* text,
        ButtonCallback callback = nullptr
    );

    void update(
        input::Input& input
    ) override;

    void draw(
        display::Display& display
    ) override;

    void setTheme(
        const Theme& theme
    ) override;

    void clear();

    int itemCount() const;
    int selectedIndex() const;

    void setSelectedIndex(int index);

    Button* item(int index);
    const Button* item(int index) const;

    void setWrapNavigation(bool enabled);

    bool wrapNavigation() const;

private:
    void selectNext();
    void selectPrevious();

    void updateScroll();

    int x_;
    int y_;
    int width_;

    int itemHeight_;
    int spacing_;

    Button items_[MAX_ITEMS];

    int itemCount_;
    int selectedIndex_;

    int scrollOffset_;

    bool wrapNavigation_;

    const Theme* theme_;
};

} // namespace ui
