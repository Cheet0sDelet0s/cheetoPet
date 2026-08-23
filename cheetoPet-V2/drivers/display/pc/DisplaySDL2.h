#pragma once

#include "../Display.h"

#include <SDL2/SDL.h>
#include <vector>

namespace display {

class DisplaySDL2 : public Display
{
public:

    explicit DisplaySDL2(
        const char* title = "Display",
        int scale = 3
    );

    ~DisplaySDL2() override;

    bool begin() override;

    void present() override;

    bool processEvents();

    bool shouldClose() const;

    SDL_Window* window() const;

    SDL_Renderer* renderer() const;

    Color* framebuffer() override;

    const Color* framebuffer() const override;

protected:

    void writePixelNative(
        int x,
        int y,
        Color color
    ) override;

    Color readPixelNative(
        int x,
        int y
    ) const override;

private:

    const char* title_;

    int scale_;

    SDL_Window* window_;

    SDL_Renderer* renderer_;

    SDL_Texture* texture_;

    std::vector<Color> framebuffer_;

    bool closeRequested_;

    bool initialized_;

    void destroySDL();

    void uploadFramebuffer();
};

} // namespace display