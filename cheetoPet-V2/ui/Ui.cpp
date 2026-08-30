#include "Ui.h"

namespace ui {

namespace {

const Theme DefaultTheme = {
    display::Color(0x0000),
    display::Color(0x7BEF),
    display::Color(0x07E0),
    display::Color(0xFFFF),
    display::Color(0x4208)
};

}

Ui::Ui(
    display::Display& display,
    input::Input& input)
    : display_(display),
      input_(input),
      screenCount_(0),
      theme_(&DefaultTheme)
{
    for (int i = 0;
         i < MAX_SCREENS;
         ++i)
    {
        screens_[i] = nullptr;
    }
}

void Ui::update()
{
    if (screenCount_ == 0)
        return;

    if (input_.wasPressed(
        input::Button::B))
    {
        pop();

        return;
    }

    screens_[screenCount_ - 1]
        ->update(input_);
}

void Ui::draw()
{
    if (screenCount_ == 0)
        return;

    Screen* screen =
        screens_[screenCount_ - 1];

    /*
     * The UI owns the background.
     */
    display_.fillScreen(
        theme_->background
    );

    screen->draw(display_);
}

bool Ui::push(Screen* screen)
{
    if (!screen)
        return false;

    if (screenCount_ >= MAX_SCREENS)
        return false;

    screens_[screenCount_] = screen;

    ++screenCount_;

    screen->setTheme(*theme_);
    screen->onEnter();

    return true;
}

bool Ui::pop()
{
    if (screenCount_ <= 1)
        return false;

    Screen* currentScreen =
        screens_[screenCount_ - 1];

    currentScreen->onExit();

    --screenCount_;

    screens_[screenCount_] = nullptr;

    return true;
}

void Ui::clear()
{
    while (screenCount_ > 0)
    {
        screens_[screenCount_ - 1]->onExit();

        --screenCount_;
    }
}

Screen* Ui::current()
{
    if (screenCount_ == 0)
        return nullptr;

    return screens_[screenCount_ - 1];
}

const Screen* Ui::current() const
{
    if (screenCount_ == 0)
        return nullptr;

    return screens_[screenCount_ - 1];
}

int Ui::screenCount() const
{
    return screenCount_;
}

void Ui::setTheme(
    const Theme& theme)
{
    theme_ = &theme;

    if (screenCount_ > 0)
    {
        screens_[screenCount_ - 1]
            ->setTheme(theme);
    }
}

const Theme& Ui::theme() const
{
    return *theme_;
}

} // namespace ui
