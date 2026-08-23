#pragma once

#include <cstdint>
#include <cstddef>
#include "Font.h"

namespace display {

using Color = uint16_t;

constexpr Color rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<Color>(
        ((r & 0xF8) << 8) |
        ((g & 0xFC) << 3) |
        (b >> 3)
    );
}

constexpr Color BLACK   = rgb565(0,   0,   0);
constexpr Color WHITE   = rgb565(255, 255, 255);
constexpr Color RED     = rgb565(255, 0,   0);
constexpr Color GREEN   = rgb565(0,   255, 0);
constexpr Color BLUE    = rgb565(0,   0,   255);
constexpr Color YELLOW  = rgb565(255, 255, 0);
constexpr Color CYAN    = rgb565(0,   255, 255);
constexpr Color MAGENTA = rgb565(255, 0,   255);
constexpr Color GRAY    = rgb565(128, 128, 128);

enum class Rotation : uint8_t
{
    Rotation0   = 0,
    Rotation90  = 1,
    Rotation180 = 2,
    Rotation270  = 3
};

class Display
{
public:
    static constexpr int NativeWidth  = 240;
    static constexpr int NativeHeight = 280;

    virtual ~Display() = default;

    virtual bool begin() = 0;
    virtual void present() = 0;

    virtual Color* framebuffer() = 0;
    virtual const Color* framebuffer() const = 0;

    // ------------------------------------------------------------
    // Display configuration
    // ------------------------------------------------------------

    void setRotation(Rotation rotation);

    Rotation getRotation() const;

    int width() const;
    int height() const;

    // ------------------------------------------------------------
    // Basic drawing
    // ------------------------------------------------------------

    void drawPixel(
        int x,
        int y,
        Color color
    );

    Color getPixel(
        int x,
        int y
    ) const;

    void fillScreen(Color color);

    void drawFastHLine(
        int x,
        int y,
        int w,
        Color color
    );

    void drawFastVLine(
        int x,
        int y,
        int h,
        Color color
    );

    void drawLine(
        int x0,
        int y0,
        int x1,
        int y1,
        Color color
    );

    void drawRect(
        int x,
        int y,
        int w,
        int h,
        Color color
    );

    void fillRect(
        int x,
        int y,
        int w,
        int h,
        Color color
    );

    void drawRoundRect(
        int x,
        int y,
        int w,
        int h,
        int radius,
        Color color
    );

    void fillRoundRect(
        int x,
        int y,
        int w,
        int h,
        int radius,
        Color color
    );

    void drawCircle(
        int x,
        int y,
        int radius,
        Color color
    );

    void fillCircle(
        int x,
        int y,
        int radius,
        Color color
    );

    void drawTriangle(
        int x0, int y0,
        int x1, int y1,
        int x2, int y2,
        Color color
    );

    void fillTriangle(
        int x0, int y0,
        int x1, int y1,
        int x2, int y2,
        Color color
    );

    // ------------------------------------------------------------
    // Images
    // ------------------------------------------------------------

    void drawBitmap(
        int x,
        int y,
        const Color* pixels,
        int w,
        int h
    );

    void pushImage(
        int x,
        int y,
        int w,
        int h,
        const Color* pixels,
        int stride = 0
    );

    void drawMonoBitmap(
        int x,
        int y,
        const uint8_t* bitmap,
        int w,
        int h,
        Color fg,
        Color bg = BLACK,
        bool transparent = false
    );

    // ------------------------------------------------------------
    // Text
    // ------------------------------------------------------------

    void setTextColor(Color color);

    void setTextColor(
        Color fg,
        Color bg
    );

    void setTextSize(uint8_t size);

    void setCursor(
        int x,
        int y
    );

    int cursorX() const;
    int cursorY() const;

    void drawChar(
        int x,
        int y,
        char c,
        Color color
    );

    void drawChar(char c);

    void print(const char* text);
    void println(const char* text);
    void print(char c);

    void print(int value);
    void print(unsigned int value);

    void print(long value);
    void print(unsigned long value);

    void print(long long value);
    void print(unsigned long long value);

    void print(float value, int decimals = 2);
    void print(double value, int decimals = 2);

    void println();

    void println(char c);

    void println(int value);
    void println(unsigned int value);

    void println(long value);
    void println(unsigned long value);

    void println(long long value);
    void println(unsigned long long value);

    void println(float value, int decimals = 2);
    void println(double value, int decimals = 2);

    void setFont(const Font& font);

    const Font& getFont() const;
    
    // ------------------------------------------------------------
    // Clipping
    // ------------------------------------------------------------

    void setClipRect(
        int x,
        int y,
        int w,
        int h
    );

    void resetClipRect();

protected:

    Display();

    /*
     * Backend-specific operations.
     *
     * Coordinates passed here are always native, unrotated
     * framebuffer coordinates.
     */
    virtual void writePixelNative(
        int x,
        int y,
        Color color
    ) = 0;

    virtual Color readPixelNative(
        int x,
        int y
    ) const = 0;

    bool logicalInBounds(
        int x,
        int y
    ) const;

    bool logicalInClip(
        int x,
        int y
    ) const;

    void logicalToNative(
        int x,
        int y,
        int& nativeX,
        int& nativeY
    ) const;

    Color textColor_;
    Color textBackground_;

    bool textBackgroundEnabled_;

    uint8_t textSize_;

    int cursorX_;
    int cursorY_;

private:

    Rotation rotation_;

    int logicalWidth_;
    int logicalHeight_;

    int clipX_;
    int clipY_;
    int clipW_;
    int clipH_;

    const Font* font_;
};

} // namespace display