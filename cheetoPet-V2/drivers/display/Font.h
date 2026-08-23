#pragma once

#include <cstdint>
#include "Font.h"

namespace display {

struct Glyph
{
    uint32_t codepoint;

    uint8_t width;
    uint8_t height;

    uint8_t advanceX;

    int8_t offsetX;
    int8_t offsetY;

    const uint8_t* bitmap;
};

struct Font
{
    uint8_t height;
    uint8_t lineSpacing;

    const Glyph* glyphs;
    uint16_t glyphCount;
};

const Glyph* findGlyph(
    const Font& font,
    uint32_t codepoint
);

} // namespace display