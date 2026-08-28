#include "Menu.h"

namespace ui {

Menu::Menu(
    int x,
    int y,
    int width,
    int itemHeight,
    int spacing)
    : theme_(nullptr),
      x_(x),
      y_(y),
      width_(width),
      itemHeight_(itemHeight),
      spacing_(spacing),
      itemCount_(0),
      selectedIndex_(-1)
{
}

bool Menu::addItem(
    const char* text,
    ButtonCallback callback)
{
    if (itemCount_ >= MAX_ITEMS)
        return false;

    const int index = itemCount_;

    const int itemY =
        y_ +
        index *
        (itemHeight_ + spacing_);

    items_[index] = Button(
        x_,
        itemY,
        width_,
        itemHeight_,
        text,
        callback
    );

    if (theme_)
    {
        items_[index].setTheme(*theme_);
    }

    ++itemCount_;

    if (selectedIndex_ == -1)
    {
        selectedIndex_ = 0;
        items_[0].onFocus();
    }

    return true;
}

void Menu::update(
    input::Input& input)
{
    if (itemCount_ == 0)
        return;

    if (input.wasPressed(
        input::Button::Up))
    {
        selectPrevious();
    }

    if (input.wasPressed(
        input::Button::Down))
    {
        selectNext();
    }

    items_[selectedIndex_].update(
        input
    );
}

void Menu::draw(
    display::Display& display)
{
    for (int i = 0;
         i < itemCount_;
         ++i)
    {
        items_[i].draw(display);
    }
}

void Menu::clear()
{
    itemCount_ = 0;
    selectedIndex_ = -1;
}

int Menu::itemCount() const
{
    return itemCount_;
}

int Menu::selectedIndex() const
{
    return selectedIndex_;
}

void Menu::setSelectedIndex(
    int index)
{
    if (index < 0 ||
        index >= itemCount_)
    {
        return;
    }

    if (selectedIndex_ >= 0)
    {
        items_[selectedIndex_].onBlur();
    }

    selectedIndex_ = index;

    items_[selectedIndex_].onFocus();
}

Button* Menu::item(int index)
{
    if (index < 0 ||
        index >= itemCount_)
    {
        return nullptr;
    }

    return &items_[index];
}

const Button* Menu::item(int index) const
{
    if (index < 0 ||
        index >= itemCount_)
    {
        return nullptr;
    }

    return &items_[index];
}

void Menu::selectNext()
{
    if (itemCount_ == 0)
        return;

    int next = selectedIndex_ + 1;

    if (next >= itemCount_)
        next = 0;

    setSelectedIndex(next);
}

void Menu::selectPrevious()
{
    if (itemCount_ == 0)
        return;

    int previous = selectedIndex_ - 1;

    if (previous < 0)
        previous = itemCount_ - 1;

    setSelectedIndex(previous);
}

void Menu::setTheme(const Theme& theme)
{
    theme_ = &theme;

    for (int i = 0; i < itemCount_; ++i)
    {
        items_[i].setTheme(theme);
    }
}

} // namespace ui