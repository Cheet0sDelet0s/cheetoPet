#pragma once

#include "../Audio.h"

#include <SDL2/SDL.h>

#include <cstdint>

namespace audio {

class AudioSDL2 : public Audio
{
public:
    AudioSDL2();
    ~AudioSDL2() override;

    bool begin() override;

    void update() override;

    void tone(
        uint16_t frequency,
        uint16_t durationMs
    ) override;

    void noTone() override;

    void setVolume(float volume) override;

    float volume() const override;

private:
    static void audioCallback(
        void* userdata,
        Uint8* stream,
        int len
    );

    void generateAudio(
        Uint8* stream,
        int len
    );

    SDL_AudioDeviceID device_;

    SDL_AudioSpec spec_;

    float volume_;

    float frequency_;

    float phase_;

    uint32_t samplesRemaining_;

    bool playing_;
};

} // namespace audio