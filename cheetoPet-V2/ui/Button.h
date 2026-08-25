#pragma once

#include "Widget.h"

namespace ui {

using ButtonCallback = void (*)();

class Button : public Widget
{
public:
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

    void setText(const char* text);
    const char* text() const;

    void setCallback(ButtonCallback callback);

    void press();

    Button();

private:
    const char* text_;
    ButtonCallback callback_;
};

} // namespace ui