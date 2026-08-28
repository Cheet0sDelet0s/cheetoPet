#include "Widget.h"

namespace ui {

namespace {

const ui::Theme DefaultTheme = {
    .background = display::Color(0x0000),
    .primary = display::Color(0x39E7),
    .focused = display::Color(0x05FF),
    .text = display::Color(0xFFFF),
    .disabled = display::Color(0x4208),
    .border = display::Color(0x7BEF),
    .focusedBorder = display::Color(0xFFFF),
    .cornerRadius = 6
};

}

Widget::Widget(
    int x,
    int y,
    int width,
    int height)
    : x_(x),
      y_(y),
      width_(width),
      height_(height),
      focused_(false),
      enabled_(true),
      theme_(&DefaultTheme)
{
}

void Widget::setTheme(const Theme& theme)
{
    theme_ = &theme;
}

const Theme& Widget::theme() const
{
    return *theme_;
}

void Widget::update(input::Input&)
{
}

void Widget::onFocus()
{
    focused_ = true;
}

void Widget::onBlur()
{
    focused_ = false;
}

bool Widget::contains(
    int x,
    int y) const
{
    return
        x >= x_ &&
        x < x_ + width_ &&
        y >= y_ &&
        y < y_ + height_;
}

void Widget::setPosition(
    int x,
    int y)
{
    x_ = x;
    y_ = y;
}

void Widget::setSize(
    int width,
    int height)
{
    width_ = width;
    height_ = height;
}

int Widget::x() const
{
    return x_;
}

int Widget::y() const
{
    return y_;
}

int Widget::width() const
{
    return width_;
}

int Widget::height() const
{
    return height_;
}

bool Widget::isFocused() const
{
    return focused_;
}

bool Widget::isEnabled() const
{
    return enabled_;
}

void Widget::setEnabled(bool enabled)
{
    enabled_ = enabled;

    if (!enabled)
        focused_ = false;
}

} // namespace ui