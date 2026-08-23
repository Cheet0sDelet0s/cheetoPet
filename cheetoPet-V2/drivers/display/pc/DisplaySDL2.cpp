#include "DisplaySDL2.h"

namespace display {

DisplaySDL2::DisplaySDL2(
    const char* title,
    int scale)
    : title_(title),
      scale_(scale < 1 ? 1 : scale),
      window_(nullptr),
      renderer_(nullptr),
      texture_(nullptr),
      framebuffer_(
          NativeWidth * NativeHeight,
          BLACK
      ),
      closeRequested_(false),
      initialized_(false)
{
}

DisplaySDL2::~DisplaySDL2()
{
    destroySDL();
}

bool DisplaySDL2::begin()
{
    if (initialized_)
        return true;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    window_ = SDL_CreateWindow(
        title_,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        NativeWidth * scale_,
        NativeHeight * scale_,
        SDL_WINDOW_SHOWN
    );

    if (!window_)
    {
        destroySDL();
        return false;
    }

    renderer_ = SDL_CreateRenderer(
        window_,
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer_)
    {
        renderer_ = SDL_CreateRenderer(
            window_,
            -1,
            SDL_RENDERER_SOFTWARE
        );
    }

    if (!renderer_)
    {
        destroySDL();
        return false;
    }

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        NativeWidth,
        NativeHeight
    );

    if (!texture_)
    {
        destroySDL();
        return false;
    }

    SDL_SetTextureBlendMode(
        texture_,
        SDL_BLENDMODE_NONE
    );

    initialized_ = true;

    fillScreen(BLACK);

    present();

    return true;
}

void DisplaySDL2::destroySDL()
{
    if (texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }

    if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    if (initialized_)
    {
        SDL_Quit();
        initialized_ = false;
    }
}

void DisplaySDL2::uploadFramebuffer()
{
    if (!texture_)
        return;

    SDL_UpdateTexture(
        texture_,
        nullptr,
        framebuffer_.data(),
        NativeWidth * sizeof(Color)
    );
}

void DisplaySDL2::present()
{
    if (!initialized_)
        return;

    uploadFramebuffer();

    SDL_RenderClear(renderer_);

    SDL_RenderCopy(
        renderer_,
        texture_,
        nullptr,
        nullptr
    );

    SDL_RenderPresent(renderer_);
}

void DisplaySDL2::writePixelNative(
    int x,
    int y,
    Color color)
{
    if (x < 0 ||
        x >= NativeWidth ||
        y < 0 ||
        y >= NativeHeight)
    {
        return;
    }

    framebuffer_[
        y * NativeWidth + x
    ] = color;
}

Color DisplaySDL2::readPixelNative(
    int x,
    int y) const
{
    if (x < 0 ||
        x >= NativeWidth ||
        y < 0 ||
        y >= NativeHeight)
    {
        return BLACK;
    }

    return framebuffer_[
        y * NativeWidth + x
    ];
}

Color* DisplaySDL2::framebuffer()
{
    return framebuffer_.data();
}

const Color* DisplaySDL2::framebuffer() const
{
    return framebuffer_.data();
}

bool DisplaySDL2::processEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                closeRequested_ = true;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    closeRequested_ = true;
                break;

            default:
                break;
        }
    }

    return !closeRequested_;
}

bool DisplaySDL2::shouldClose() const
{
    return closeRequested_;
}

SDL_Window* DisplaySDL2::window() const
{
    return window_;
}

SDL_Renderer* DisplaySDL2::renderer() const
{
    return renderer_;
}

} // namespace display