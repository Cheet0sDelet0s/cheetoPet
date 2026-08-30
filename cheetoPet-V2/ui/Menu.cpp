#include "Menu.h"

#include <algorithm>

namespace ui {

Menu::Menu(
    int x,
    int y,
    int width,
    int height,
    int itemHeight,
    int spacing)
    : x_(x),
      y_(y),
      width_(width),
      itemHeight_(itemHeight),
      spacing_(spacing),
      itemCount_(0),
      selectedIndex_(-1),
      scrollOffset_(0),
      wrapNavigation_(true),
      theme_(nullptr)
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
        items_[index].setTheme(*theme_);

    ++itemCount_;

    if (selectedIndex_ == -1)
    {
        selectedIndex_ = 0;
        items_[0].onFocus();
    }

    updateScroll();

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

    /*
     * B acts as "back".
     *
     * The actual pop is handled by Ui.
     * We don't do it here because Menu doesn't know
     * which Ui instance owns it.
     */

    if (selectedIndex_ >= 0)
    {
        items_[selectedIndex_].update(input);
    }
}

void Menu::draw(
    display::Display& display)
{
    if (itemCount_ == 0)
        return;

    /*
     * Determine which items are visible.
     */
    const int visibleHeight =
        display.height() - y_;

    const int itemStep =
        itemHeight_ + spacing_;

    const int visibleItems =
        (visibleHeight + spacing_) /
        itemStep;

    const int firstItem =
        scrollOffset_;

    const int lastItem =
        std::min(
            itemCount_,
            firstItem + visibleItems
        );

    /*
     * Position and draw only visible items.
     */
    for (int i = firstItem;
         i < lastItem;
         ++i)
    {
        const int itemY =
            y_ +
            (i - scrollOffset_) *
            itemStep;

        items_[i].setPosition(
            x_,
            itemY
        );

        items_[i].draw(display);
    }
}

void Menu::setTheme(
    const Theme& theme)
{
    theme_ = &theme;

    for (int i = 0;
         i < itemCount_;
         ++i)
    {
        items_[i].setTheme(theme);
    }
}

void Menu::clear()
{
    itemCount_ = 0;
    selectedIndex_ = -1;
    scrollOffset_ = 0;
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

    updateScroll();
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

void Menu::setWrapNavigation(
    bool enabled)
{
    wrapNavigation_ = enabled;
}

bool Menu::wrapNavigation() const
{
    return wrapNavigation_;
}

void Menu::selectNext()
{
    if (itemCount_ == 0)
        return;

    int next =
        selectedIndex_ + 1;

    if (next >= itemCount_)
    {
        if (wrapNavigation_)
            next = 0;
        else
            next = itemCount_ - 1;
    }

    setSelectedIndex(next);
}

void Menu::selectPrevious()
{
    if (itemCount_ == 0)
        return;

    int previous =
        selectedIndex_ - 1;

    if (previous < 0)
    {
        if (wrapNavigation_)
            previous = itemCount_ - 1;
        else
            previous = 0;
    }

    setSelectedIndex(previous);
}

void Menu::updateScroll()
{
    if (selectedIndex_ < 0)
        return;

    const int itemStep =
        itemHeight_ + spacing_;

    const int visibleItems =
        std::max(
            1,
            (itemHeight_ + spacing_) /
            itemStep
        );

    if (selectedIndex_ < scrollOffset_)
    {
        scrollOffset_ = selectedIndex_;
    }

    if (selectedIndex_ >=
        scrollOffset_ + visibleItems)
    {
        scrollOffset_ =
            selectedIndex_ -
            visibleItems +
            1;
    }

    const int maxScroll =
        std::max(
            0,
            itemCount_ - visibleItems
        );

    scrollOffset_ =
        std::min(
            scrollOffset_,
            maxScroll
        );
}

} // namespace ui