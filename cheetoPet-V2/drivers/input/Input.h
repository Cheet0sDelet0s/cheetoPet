#pragma once

#include <cstdint>

namespace input {

enum class Button : uint8_t
{
    A = 0,
    B,
    X,
    Up,
    Down,
    Select,
    Power,

    Count
};

class Input
{
public:
    virtual ~Input() = default;

    virtual bool begin() = 0;
    virtual void update() = 0;

    // True while the button is held.
    virtual bool isPressed(Button button) const = 0;

    // True for one update cycle when the button has just
    // transitioned from released -> pressed.
    virtual bool wasPressed(Button button) const = 0;

    // True for one update cycle when the button has just
    // transitioned from pressed -> released.
    virtual bool wasReleased(Button button) const = 0;
};

} // namespace input