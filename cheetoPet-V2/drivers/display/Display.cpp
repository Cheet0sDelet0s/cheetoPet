#include "Display.h"
#include "Font.h"
#include "fonts/Font5x7.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>

namespace display {

Display::Display()
    : textColor_(WHITE),
      textBackground_(BLACK),
      textBackgroundEnabled_(false),
      textSize_(1),
      cursorX_(0),
      cursorY_(0),
      rotation_(Rotation::Rotation0),
      logicalWidth_(NativeWidth),
      logicalHeight_(NativeHeight),
      clipX_(0),
      clipY_(0),
      clipW_(NativeWidth),
      clipH_(NativeHeight),
      font_(&fonts::Font5x7)
{
}

// ================================================================
// Configuration
// ================================================================

int Display::width() const
{
    return logicalWidth_;
}

int Display::height() const
{
    return logicalHeight_;
}

Rotation Display::getRotation() const
{
    return rotation_;
}

void Display::setRotation(Rotation rotation)
{
    rotation_ = rotation;

    switch (rotation_)
    {
        case Rotation::Rotation0:
        case Rotation::Rotation180:
            logicalWidth_ = NativeWidth;
            logicalHeight_ = NativeHeight;
            break;

        case Rotation::Rotation90:
        case Rotation::Rotation270:
            logicalWidth_ = NativeHeight;
            logicalHeight_ = NativeWidth;
            break;
    }

    resetClipRect();

    cursorX_ = 0;
    cursorY_ = 0;
}

// ================================================================
// Coordinate handling
// ================================================================

bool Display::logicalInBounds(
    int x,
    int y) const
{
    return
        x >= 0 &&
        y >= 0 &&
        x < logicalWidth_ &&
        y < logicalHeight_;
}

bool Display::logicalInClip(
    int x,
    int y) const
{
    return
        x >= clipX_ &&
        y >= clipY_ &&
        x < clipX_ + clipW_ &&
        y < clipY_ + clipH_;
}

void Display::logicalToNative(
    int x,
    int y,
    int& nativeX,
    int& nativeY) const
{
    switch (rotation_)
    {
        case Rotation::Rotation0:
            nativeX = x;
            nativeY = y;
            break;

        case Rotation::Rotation90:
            nativeX = NativeWidth - 1 - y;
            nativeY = x;
            break;

        case Rotation::Rotation180:
            nativeX = NativeWidth - 1 - x;
            nativeY = NativeHeight - 1 - y;
            break;

        case Rotation::Rotation270:
            nativeX = y;
            nativeY = NativeHeight - 1 - x;
            break;
    }
}

// ================================================================
// Pixels
// ================================================================

void Display::drawPixel(
    int x,
    int y,
    Color color)
{
    if (!logicalInBounds(x, y))
        return;

    if (!logicalInClip(x, y))
        return;

    int nativeX;
    int nativeY;

    logicalToNative(
        x,
        y,
        nativeX,
        nativeY
    );

    writePixelNative(
        nativeX,
        nativeY,
        color
    );
}

Color Display::getPixel(
    int x,
    int y) const
{
    if (!logicalInBounds(x, y))
        return BLACK;

    int nativeX;
    int nativeY;

    logicalToNative(
        x,
        y,
        nativeX,
        nativeY
    );

    return readPixelNative(
        nativeX,
        nativeY
    );
}

void Display::fillScreen(Color color)
{
    Color* fb = framebuffer();

    if (!fb)
        return;

    std::fill(
        fb,
        fb + NativeWidth * NativeHeight,
        color
    );
}

// ================================================================
// Lines
// ================================================================

void Display::drawFastHLine(
    int x,
    int y,
    int w,
    Color color)
{
    if (w < 0)
    {
        x += w + 1;
        w = -w;
    }

    if (w <= 0)
        return;

    for (int i = 0; i < w; ++i)
        drawPixel(x + i, y, color);
}

void Display::drawFastVLine(
    int x,
    int y,
    int h,
    Color color)
{
    if (h < 0)
    {
        y += h + 1;
        h = -h;
    }

    if (h <= 0)
        return;

    for (int i = 0; i < h; ++i)
        drawPixel(x, y + i, color);
}

void Display::drawLine(
    int x0,
    int y0,
    int x1,
    int y1,
    Color color)
{
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;

    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;

    int error = dx + dy;

    while (true)
    {
        drawPixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        const int e2 = error * 2;

        if (e2 >= dy)
        {
            error += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}

// ================================================================
// Rectangles
// ================================================================

void Display::drawRect(
    int x,
    int y,
    int w,
    int h,
    Color color)
{
    if (w <= 0 || h <= 0)
        return;

    drawFastHLine(
        x,
        y,
        w,
        color
    );

    drawFastHLine(
        x,
        y + h - 1,
        w,
        color
    );

    drawFastVLine(
        x,
        y,
        h,
        color
    );

    drawFastVLine(
        x + w - 1,
        y,
        h,
        color
    );
}

void Display::fillRect(
    int x,
    int y,
    int w,
    int h,
    Color color)
{
    if (w <= 0 || h <= 0)
        return;

    for (int yy = y; yy < y + h; ++yy)
    {
        drawFastHLine(
            x,
            yy,
            w,
            color
        );
    }
}

// ================================================================
// Circles
// ================================================================

void Display::drawCircle(
    int x0,
    int y0,
    int radius,
    Color color)
{
    if (radius < 0)
        return;

    if (radius == 0)
    {
        drawPixel(x0, y0, color);
        return;
    }

    int x = radius;
    int y = 0;

    int decision = 1 - radius;

    while (x >= y)
    {
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);

        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);

        ++y;

        if (decision <= 0)
        {
            decision += 2 * y + 1;
        }
        else
        {
            --x;
            decision += 2 * (y - x) + 1;
        }
    }
}

void Display::fillCircle(
    int x0,
    int y0,
    int radius,
    Color color)
{
    if (radius < 0)
        return;

    if (radius == 0)
    {
        drawPixel(x0, y0, color);
        return;
    }

    /*
     * Scanline implementation.
     *
     * Every horizontal row is explicitly filled from the left
     * edge of the circle to the right edge. This avoids the gaps
     * produced by the previous midpoint-based implementation.
     */
    const int radiusSquared = radius * radius;

    for (int dy = -radius; dy <= radius; ++dy)
    {
        const int yy = dy * dy;

        const int remaining =
            radiusSquared - yy;

        const int dx = static_cast<int>(
            std::sqrt(
                static_cast<double>(remaining)
            )
        );

        drawFastHLine(
            x0 - dx,
            y0 + dy,
            2 * dx + 1,
            color
        );
    }
}

// ================================================================
// Rounded rectangles
// ================================================================

void Display::drawRoundRect(
    int x,
    int y,
    int w,
    int h,
    int radius,
    Color color)
{
    if (w <= 0 || h <= 0)
        return;

    radius = std::max(
        0,
        std::min(radius, std::min(w, h) / 2)
    );

    if (radius == 0)
    {
        drawRect(
            x,
            y,
            w,
            h,
            color
        );

        return;
    }

    // Straight sections
    drawFastHLine(
        x + radius,
        y,
        w - 2 * radius,
        color
    );

    drawFastHLine(
        x + radius,
        y + h - 1,
        w - 2 * radius,
        color
    );

    drawFastVLine(
        x,
        y + radius,
        h - 2 * radius,
        color
    );

    drawFastVLine(
        x + w - 1,
        y + radius,
        h - 2 * radius,
        color
    );

    /*
     * Rather than relying on helper functions, draw the four
     * corner arcs directly.
     */
    const int cx1 = x + radius;
    const int cx2 = x + w - radius - 1;

    const int cy1 = y + radius;
    const int cy2 = y + h - radius - 1;

    int px = radius;
    int py = 0;
    int decision = 1 - radius;

    while (px >= py)
    {
        // Top-left
        drawPixel(cx1 - px, cy1 - py, color);
        drawPixel(cx1 - py, cy1 - px, color);

        // Top-right
        drawPixel(cx2 + px, cy1 - py, color);
        drawPixel(cx2 + py, cy1 - px, color);

        // Bottom-left
        drawPixel(cx1 - px, cy2 + py, color);
        drawPixel(cx1 - py, cy2 + px, color);

        // Bottom-right
        drawPixel(cx2 + px, cy2 + py, color);
        drawPixel(cx2 + py, cy2 + px, color);

        ++py;

        if (decision <= 0)
        {
            decision += 2 * py + 1;
        }
        else
        {
            --px;
            decision += 2 * (py - px) + 1;
        }
    }
}

void Display::fillRoundRect(
    int x,
    int y,
    int w,
    int h,
    int radius,
    Color color)
{
    if (w <= 0 || h <= 0)
        return;

    radius = std::max(
        0,
        std::min(radius, std::min(w, h) / 2)
    );

    if (radius == 0)
    {
        fillRect(
            x,
            y,
            w,
            h,
            color
        );

        return;
    }

    /*
     * Fill the centre rectangle first.
     */
    fillRect(
        x + radius,
        y,
        w - 2 * radius,
        h,
        color
    );

    /*
     * Then fill each scanline of the rounded ends.
     *
     * This deliberately uses the same reliable scanline method
     * as fillCircle().
     */
    const int radiusSquared = radius * radius;

    for (int dy = -radius; dy <= radius; ++dy)
    {
        const int yy = dy * dy;

        const int remaining =
            radiusSquared - yy;

        const int dx = static_cast<int>(
            std::sqrt(
                static_cast<double>(remaining)
            )
        );

        const int lineY = y + radius + dy;

        drawFastHLine(
            x + radius - dx,
            lineY,
            w - 2 * radius + 2 * dx,
            color
        );

        const int bottomY =
            y + h - radius - 1 - dy;

        drawFastHLine(
            x + radius - dx,
            bottomY,
            w - 2 * radius + 2 * dx,
            color
        );
    }
}

// ================================================================
// Triangles
// ================================================================

void Display::drawTriangle(
    int x0, int y0,
    int x1, int y1,
    int x2, int y2,
    Color color)
{
    drawLine(
        x0, y0,
        x1, y1,
        color
    );

    drawLine(
        x1, y1,
        x2, y2,
        color
    );

    drawLine(
        x2, y2,
        x0, y0,
        color
    );
}

void Display::fillTriangle(
    int x0, int y0,
    int x1, int y1,
    int x2, int y2,
    Color color)
{
    if (y0 > y1)
    {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    if (y1 > y2)
    {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }

    if (y0 > y1)
    {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    if (y0 == y2)
    {
        const int minX =
            std::min({x0, x1, x2});

        const int maxX =
            std::max({x0, x1, x2});

        drawFastHLine(
            minX,
            y0,
            maxX - minX + 1,
            color
        );

        return;
    }

    const int dx01 = x1 - x0;
    const int dy01 = y1 - y0;

    const int dx02 = x2 - x0;
    const int dy02 = y2 - y0;

    const int dx12 = x2 - x1;
    const int dy12 = y2 - y1;

    int sa = 0;
    int sb = 0;

    const int last =
        (y1 == y2) ? y1 : y1 - 1;

    for (int y = y0; y <= last; ++y)
    {
        const int a =
            x0 + sa / dy01;

        const int b =
            x0 + sb / dy02;

        sa += dx01;
        sb += dx02;

        if (a < b)
        {
            drawFastHLine(
                a,
                y,
                b - a + 1,
                color
            );
        }
        else
        {
            drawFastHLine(
                b,
                y,
                a - b + 1,
                color
            );
        }
    }

    sa = dx12 * (last + 1 - y1);
    sb = dx02 * (last + 1 - y0);

    for (int y = last + 1; y <= y2; ++y)
    {
        const int a =
            x1 + sa / dy12;

        const int b =
            x0 + sb / dy02;

        sa += dx12;
        sb += dx02;

        if (a < b)
        {
            drawFastHLine(
                a,
                y,
                b - a + 1,
                color
            );
        }
        else
        {
            drawFastHLine(
                b,
                y,
                a - b + 1,
                color
            );
        }
    }
}

// ================================================================
// Images
// ================================================================

void Display::drawBitmap(
    int x,
    int y,
    const Color* pixels,
    int w,
    int h)
{
    pushImage(
        x,
        y,
        w,
        h,
        pixels,
        w
    );
}

void Display::pushImage(
    int x,
    int y,
    int w,
    int h,
    const Color* pixels,
    int stride)
{
    if (!pixels || w <= 0 || h <= 0)
        return;

    if (stride <= 0)
        stride = w;

    for (int yy = 0; yy < h; ++yy)
    {
        for (int xx = 0; xx < w; ++xx)
        {
            drawPixel(
                x + xx,
                y + yy,
                pixels[yy * stride + xx]
            );
        }
    }
}

void Display::drawMonoBitmap(
    int x,
    int y,
    const uint8_t* bitmap,
    int w,
    int h,
    Color fg,
    Color bg,
    bool transparent)
{
    if (!bitmap || w <= 0 || h <= 0)
        return;

    const int bytesPerRow =
        (w + 7) / 8;

    for (int yy = 0; yy < h; ++yy)
    {
        for (int xx = 0; xx < w; ++xx)
        {
            const uint8_t byte =
                bitmap[
                    yy * bytesPerRow +
                    xx / 8
                ];

            const bool set =
                (byte & (0x80 >> (xx & 7))) != 0;

            if (set)
            {
                drawPixel(
                    x + xx,
                    y + yy,
                    fg
                );
            }
            else if (!transparent)
            {
                drawPixel(
                    x + xx,
                    y + yy,
                    bg
                );
            }
        }
    }
}

// ================================================================
// Text
// ================================================================

void Display::setTextColor(Color color)
{
    textColor_ = color;
    textBackgroundEnabled_ = false;
}

void Display::setTextColor(
    Color fg,
    Color bg)
{
    textColor_ = fg;
    textBackground_ = bg;
    textBackgroundEnabled_ = true;
}

void Display::setTextSize(uint8_t size)
{
    textSize_ =
        std::max<uint8_t>(1, size);
}

void Display::setCursor(
    int x,
    int y)
{
    cursorX_ = x;
    cursorY_ = y;
}

int Display::cursorX() const
{
    return cursorX_;
}

int Display::cursorY() const
{
    return cursorY_;
}

void Display::drawChar(
    int x,
    int y,
    char c,
    Color color)
{
    if (!font_)
        return;

    const Glyph* glyph =
        findGlyph(
            *font_,
            static_cast<uint8_t>(c)
        );

    if (!glyph)
        return;

    for (int row = 0;
         row < glyph->height;
         ++row)
    {
        const uint8_t bits =
            glyph->bitmap[row];

        for (int column = 0;
             column < glyph->width;
             ++column)
        {
            if (bits & (1 << (glyph->width - 1 - column)))
            {
                fillRect(
                    x +
                    (glyph->offsetX + column) *
                        textSize_,

                    y +
                    (glyph->offsetY + row) *
                        textSize_,

                    textSize_,
                    textSize_,
                    color
                );
            }
        }
    }
}

void Display::drawChar(char c)
{
    if (!font_)
        return;

    if (c == '\n')
    {
        cursorX_ = 0;

        cursorY_ +=
            (font_->height +
             font_->lineSpacing) *
            textSize_;

        return;
    }

    if (c == '\r')
        return;

    const Glyph* glyph =
        findGlyph(
            *font_,
            static_cast<uint8_t>(c)
        );

    if (!glyph)
        return;

    if (textBackgroundEnabled_)
    {
        fillRect(
            cursorX_,
            cursorY_,
            glyph->advanceX * textSize_,
            font_->height * textSize_,
            textBackground_
        );
    }

    drawChar(
        cursorX_,
        cursorY_,
        c,
        textColor_
    );

    cursorX_ +=
        glyph->advanceX * textSize_;
}

void Display::print(
    const char* text)
{
    if (!text)
        return;

    while (*text)
        drawChar(*text++);
}

void Display::print(char c)
{
    drawChar(c);
}

void Display::print(int value)
{
    char buffer[32];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%d",
        value
    );

    print(buffer);
}

void Display::print(unsigned int value)
{
    char buffer[32];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%u",
        value
    );

    print(buffer);
}

void Display::print(long value)
{
    char buffer[32];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%ld",
        value
    );

    print(buffer);
}

void Display::print(unsigned long value)
{
    char buffer[32];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%lu",
        value
    );

    print(buffer);
}

void Display::print(long long value)
{
    char buffer[32];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%lld",
        value
    );

    print(buffer);
}

void Display::print(unsigned long long value)
{
    char buffer[32];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%llu",
        value
    );

    print(buffer);
}

void Display::print(
    float value,
    int decimals)
{
    char buffer[64];

    if (decimals < 0)
        decimals = 0;

    if (decimals > 9)
        decimals = 9;

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%.*f",
        decimals,
        static_cast<double>(value)
    );

    print(buffer);
}

void Display::print(
    double value,
    int decimals)
{
    char buffer[64];

    if (decimals < 0)
        decimals = 0;

    if (decimals > 9)
        decimals = 9;

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%.*f",
        decimals,
        value
    );

    print(buffer);
}

void Display::setFont(const Font& font)
{
    font_ = &font;
}

const Font& Display::getFont() const
{
    return *font_;
}

// ================================================================
// println()
// ================================================================

void Display::println()
{
    drawChar('\n');
}

void Display::println(
    const char* text)
{
    print(text);
    drawChar('\n');
}

void Display::println(char c)
{
    print(c);
    drawChar('\n');
}

void Display::println(int value)
{
    print(value);
    drawChar('\n');
}

void Display::println(unsigned int value)
{
    print(value);
    drawChar('\n');
}

void Display::println(long value)
{
    print(value);
    drawChar('\n');
}

void Display::println(unsigned long value)
{
    print(value);
    drawChar('\n');
}

void Display::println(long long value)
{
    print(value);
    drawChar('\n');
}

void Display::println(unsigned long long value)
{
    print(value);
    drawChar('\n');
}

void Display::println(
    float value,
    int decimals)
{
    print(value, decimals);
    drawChar('\n');
}

void Display::println(
    double value,
    int decimals)
{
    print(value, decimals);
    drawChar('\n');
}

// ================================================================
// Clipping
// ================================================================

void Display::setClipRect(
    int x,
    int y,
    int w,
    int h)
{
    const int x1 =
        std::max(0, x);

    const int y1 =
        std::max(0, y);

    const int x2 =
        std::min(logicalWidth_, x + w);

    const int y2 =
        std::min(logicalHeight_, y + h);

    clipX_ = x1;
    clipY_ = y1;

    clipW_ =
        std::max(0, x2 - x1);

    clipH_ =
        std::max(0, y2 - y1);
}

void Display::resetClipRect()
{
    clipX_ = 0;
    clipY_ = 0;
    clipW_ = logicalWidth_;
    clipH_ = logicalHeight_;
}

} // namespace display