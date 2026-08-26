#include "Button.h"

namespace ui {

Button::Button()
    : Widget(0, 0, 0, 0),
      text_(nullptr),
      callback_(nullptr)
{
}

Button::Button(
    int x,
    int y,
    int width,
    int height,
    const char* text,
    ButtonCallback callback)
    : Widget(
        x,
        y,
        width,
        height
      ),
      text_(text),
      callback_(callback)
{
}

void Button::update(
    input::Input& input)
{
    if (!enabled_)
        return;

    if (focused_ &&
        input.wasPressed(input::Button::Select))
    {
        press();
    }
}

void Button::draw(
    display::Display& display)
{
    display::Color background;

    if (!enabled_)
    {
        background = background_color;
    }
    else if (focused_)
    {
        background = COLOR_FOCUSED;
    }
    else
    {
        background = COLOR_BUTTON;
    }

    display.fillRoundRect(
        x_,
        y_,
        width_,
        height_,
        5,
        background
    );

    display.setTextColor(
        COLOR_TEXT
    );

    display.setTextSize(1);

    // center text

    int textWidth = display.textWidth(text_);

    int textX =
        x_ + (width_ - textWidth) / 2;

    const int textY =
        y_ + (height_ - display.fontHeight()) / 2;

    display.setCursor(
        textX,
        textY
    );

    if (text_)
        display.print(text_);
}

void Button::setText(
    const char* text)
{
    text_ = text;
}

const char* Button::text() const
{
    return text_;
}

void Button::setCallback(
    ButtonCallback callback)
{
    callback_ = callback;
}

void Button::press()
{
    if (callback_)
        callback_();
}

} // namespace ui