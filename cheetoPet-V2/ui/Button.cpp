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
    const Theme& t = theme();

    display::Color background;
    display::Color border;

    if (!enabled_)
    {
        background = t.disabled;
        border = t.border; 
    }
    else if (focused_)
    {
        background = t.focused;
        border = t.focusedBorder;
    }
    else
    {
        background = t.primary;
        border = t.border;
    }

    display.fillRoundRect(
        x_,
        y_,
        width_,
        height_,
        t.cornerRadius,
        background
    );

    display.drawRoundRect(
        x_,
        y_,
        width_,
        height_,
        t.cornerRadius,
        border
    );

    display.setTextColor(
        t.text
    );

    display.setTextSize(1);

    const int textWidth =
        display.textWidth(text_);

    const int textX =
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