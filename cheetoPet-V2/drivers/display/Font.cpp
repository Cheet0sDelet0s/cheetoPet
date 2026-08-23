#include "Font.h"

namespace display {

const Glyph* findGlyph(
    const Font& font,
    uint32_t codepoint)
{
    for (uint16_t i = 0; i < font.glyphCount; ++i)
    {
        if (font.glyphs[i].codepoint == codepoint)
            return &font.glyphs[i];
    }

    return nullptr;
}

} // namespace display