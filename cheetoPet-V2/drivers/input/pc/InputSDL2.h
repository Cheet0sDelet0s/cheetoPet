#pragma once

#include "../Input.h"

#include <SDL2/SDL.h>

namespace input {

class InputSDL2 : public Input
{
public:
    InputSDL2();

    bool begin() override;
    void update() override;

    bool isPressed(Button button) const override;
    bool wasPressed(Button button) const override;
    bool wasReleased(Button button) const override;

    void setKey(Button button, SDL_Scancode key);

private:
    SDL_Scancode keys[
        static_cast<int>(Button::Count)
    ];

    bool current[
        static_cast<int>(Button::Count)
    ];

    bool previous[
        static_cast<int>(Button::Count)
    ];
};

} // namespace input