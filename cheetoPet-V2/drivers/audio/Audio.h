#pragma once

#include <cstdint>
#include <cstddef>

namespace audio {

class Audio
{
public:
    virtual ~Audio() = default;

    virtual bool begin() = 0;

    virtual void update() {}

    // Start/stop a simple tone.
    virtual void tone(
        uint16_t frequency,
        uint16_t durationMs
    ) = 0;

    virtual void noTone() = 0;

    // Set master volume, 0.0 - 1.0.
    virtual void setVolume(float volume) = 0;

    virtual float volume() const = 0;
};

} // namespace audio