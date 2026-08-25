#include "AudioSDL2.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace audio {

namespace {

constexpr float PI = 3.14159265358979323846f;

}

AudioSDL2::AudioSDL2()
    : device_(0),
      spec_{},
      volume_(1.0f),
      frequency_(0.0f),
      phase_(0.0f),
      samplesRemaining_(0),
      playing_(false)
{
}

AudioSDL2::~AudioSDL2()
{
    noTone();

    if (device_ != 0)
    {
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }
}

bool AudioSDL2::begin()
{
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO))
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
            return false;
    }

    SDL_AudioSpec desired{};

    desired.freq = 44100;
    desired.format = AUDIO_F32SYS;
    desired.channels = 1;
    desired.samples = 1024;
    desired.callback = audioCallback;
    desired.userdata = this;

    device_ = SDL_OpenAudioDevice(
        nullptr,
        0,
        &desired,
        &spec_,
        0
    );

    if (device_ == 0)
        return false;

    SDL_PauseAudioDevice(
        device_,
        0
    );

    return true;
}

void AudioSDL2::update()
{
    // Tone playback is handled by the SDL audio callback.
}

void AudioSDL2::tone(
    uint16_t frequency,
    uint16_t durationMs)
{
    if (device_ == 0)
        return;

    SDL_LockAudioDevice(device_);

    frequency_ =
        static_cast<float>(frequency);

    phase_ = 0.0f;

    samplesRemaining_ =
        static_cast<uint32_t>(
            static_cast<uint64_t>(
                spec_.freq
            ) * durationMs / 1000
        );

    playing_ = true;

    SDL_UnlockAudioDevice(device_);
}

void AudioSDL2::noTone()
{
    if (device_ == 0)
        return;

    SDL_LockAudioDevice(device_);

    playing_ = false;
    samplesRemaining_ = 0;

    SDL_UnlockAudioDevice(device_);
}

void AudioSDL2::setVolume(float volume)
{
    volume_ = std::clamp(
        volume,
        0.0f,
        1.0f
    );
}

float AudioSDL2::volume() const
{
    return volume_;
}

void AudioSDL2::audioCallback(
    void* userdata,
    Uint8* stream,
    int len)
{
    auto* audio =
        static_cast<AudioSDL2*>(userdata);

    audio->generateAudio(
        stream,
        len
    );
}

void AudioSDL2::generateAudio(
    Uint8* stream,
    int len)
{
    auto* output =
        reinterpret_cast<float*>(stream);

    const int sampleCount =
        len / sizeof(float);

    for (int i = 0;
         i < sampleCount;
         ++i)
    {
        if (!playing_ ||
            samplesRemaining_ == 0)
        {
            output[i] = 0.0f;
            continue;
        }

        const float sample =
            std::sin(phase_) *
            0.25f *
            volume_;

        output[i] = sample;

        phase_ +=
            2.0f *
            PI *
            frequency_ /
            static_cast<float>(spec_.freq);

        if (phase_ >= 2.0f * PI)
            phase_ -= 2.0f * PI;

        --samplesRemaining_;

        if (samplesRemaining_ == 0)
            playing_ = false;
    }
}

} // namespace audio