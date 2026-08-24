#include "InputSDL2.h"

namespace input {

InputSDL2::InputSDL2()
{
    for (int i = 0;
         i < static_cast<int>(Button::Count);
         ++i)
    {
        keys[i] = SDL_SCANCODE_UNKNOWN;
        current[i] = false;
        previous[i] = false;
    }
}

bool InputSDL2::begin()
{
    if (!(SDL_WasInit(SDL_INIT_EVENTS) & SDL_INIT_EVENTS))
    {
        if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
            return false;
    }

    InputSDL2::setKey(
        input::Button::Up,
        SDL_SCANCODE_UP
    );

    InputSDL2::setKey(
        input::Button::Down,
        SDL_SCANCODE_DOWN
    );

    InputSDL2::setKey(
        input::Button::A,
        SDL_SCANCODE_Z
    );

    InputSDL2::setKey(
        input::Button::X,
        SDL_SCANCODE_X
    );

    InputSDL2::setKey(
        input::Button::B,
        SDL_SCANCODE_C
    );

    InputSDL2::setKey(
        input::Button::Select,
        SDL_SCANCODE_SPACE
    );

    InputSDL2::setKey(
        input::Button::Power,
        SDL_SCANCODE_P
    );

    return true;
}

void InputSDL2::update()
{
    for (int i = 0;
         i < static_cast<int>(Button::Count);
         ++i)
    {
        previous[i] = current[i];
    }

    SDL_PumpEvents();

    const Uint8* keyboard = SDL_GetKeyboardState(nullptr);

    for (int i = 0;
         i < static_cast<int>(Button::Count);
         ++i)
    {
        if (keys[i] == SDL_SCANCODE_UNKNOWN)
            current[i] = false;
        else
            current[i] = keyboard[keys[i]];
    }
}

bool InputSDL2::isPressed(Button button) const
{
    return current[
        static_cast<int>(button)
    ];
}

bool InputSDL2::wasPressed(Button button) const
{
    const int i = static_cast<int>(button);

    return current[i] && !previous[i];
}

bool InputSDL2::wasReleased(Button button) const
{
    const int i = static_cast<int>(button);

    return !current[i] && previous[i];
}

void InputSDL2::setKey(
    Button button,
    SDL_Scancode key)
{
    keys[
        static_cast<int>(button)
    ] = key;
}

} // namespace input