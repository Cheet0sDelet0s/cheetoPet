#include "Ui.h"

namespace ui {

Ui::Ui(
    display::Display& display,
    input::Input& input)
    : display_(display),
      input_(input),
      menu_(nullptr)
{
}

void Ui::update()
{
    if (menu_)
        menu_->update(input_);
}

void Ui::draw()
{
    if (menu_)
        menu_->draw(display_);
}

void Ui::setMenu(
    Menu* menu)
{
    menu_ = menu;
}

Menu* Ui::menu()
{
    return menu_;
}

const Menu* Ui::menu() const
{
    return menu_;
}

} // namespace ui