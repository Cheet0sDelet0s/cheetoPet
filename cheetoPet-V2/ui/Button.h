#pragma once

#include "Widget.h"

namespace ui {

class Ui;

using ButtonCallback = void (*)(Ui&);

class Button : public Widget
{
public:
    Button();

    Button(
        int x,
        int y,
        int width,
        int height,
        const char* text,
        ButtonCallback callback = nullptr
    );

    void update(
        input::Input& input
    ) override;

    void draw(
        display::Display& display
    ) override;

    void setText(
        const char* text
    );

    const char* text() const;

    void setCallback(
        ButtonCallback callback
    );

    void press(
        Ui& ui
    );

private:
    const char* text_;
    ButtonCallback callback_;
};

} // namespace ui