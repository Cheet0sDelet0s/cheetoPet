#pragma once

#include "../drivers/display/Display.h"
#include "../drivers/input/Input.h"
#include "Theme.h"

namespace ui {

class Widget
{
public:
    Widget(
        int x,
        int y,
        int width,
        int height
    );

    virtual ~Widget() = default;

    virtual void update(
        input::Input& input
    );

    virtual void draw(
        display::Display& display
    ) = 0;

    virtual void onFocus();
    virtual void onBlur();

    bool contains(
        int x,
        int y
    ) const;

    void setPosition(
        int x,
        int y
    );

    void setSize(
        int width,
        int height
    );

    void setTheme(const Theme& theme);
    const Theme& theme() const;

    int x() const;
    int y() const;
    int width() const;
    int height() const;

    bool isFocused() const;
    bool isEnabled() const;

    void setEnabled(bool enabled);

protected:
    int x_;
    int y_;
    int width_;
    int height_;

    bool focused_;
    bool enabled_;

    const Theme* theme_;
};

} // namespace ui