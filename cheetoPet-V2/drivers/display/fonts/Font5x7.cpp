#include "Font5x7.h"

namespace display {
namespace fonts {

namespace {

#define GLYPH(name, ...) \
    static const uint8_t name[] = { __VA_ARGS__ }

GLYPH(glyph_space,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
);

GLYPH(glyph_A,
    0b01110,
    0b10001,
    0b10001,
    0b11111,
    0b10001,
    0b10001,
    0b10001
);

GLYPH(glyph_B,
    0b11110,
    0b10001,
    0b10001,
    0b11110,
    0b10001,
    0b10001,
    0b11110
);

GLYPH(glyph_C,
    0b01111,
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b01111
);

GLYPH(glyph_D,
    0b11110,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11110
);

GLYPH(glyph_E,
    0b11111,
    0b10000,
    0b10000,
    0b11110,
    0b10000,
    0b10000,
    0b11111
);

GLYPH(glyph_F,
    0b11111,
    0b10000,
    0b10000,
    0b11110,
    0b10000,
    0b10000,
    0b10000
);

GLYPH(glyph_G,
    0b01111,
    0b10000,
    0b10000,
    0b10111,
    0b10001,
    0b10001,
    0b01111
);

GLYPH(glyph_H,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b10001,
    0b10001,
    0b10001
);

GLYPH(glyph_I,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b11111
);

GLYPH(glyph_J,
    0b00111,
    0b00010,
    0b00010,
    0b00010,
    0b00010,
    0b10010,
    0b01100
);

GLYPH(glyph_K,
    0b10001,
    0b10010,
    0b10100,
    0b11000,
    0b10100,
    0b10010,
    0b10001
);

GLYPH(glyph_L,
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b11111
);

GLYPH(glyph_M,
    0b10001,
    0b11011,
    0b10101,
    0b10101,
    0b10001,
    0b10001,
    0b10001
);

GLYPH(glyph_N,
    0b10001,
    0b11001,
    0b10101,
    0b10011,
    0b10001,
    0b10001,
    0b10001
);

GLYPH(glyph_O,
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b01110
);

GLYPH(glyph_P,
    0b11110,
    0b10001,
    0b10001,
    0b11110,
    0b10000,
    0b10000,
    0b10000
);

GLYPH(glyph_Q,
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b10101,
    0b10010,
    0b01101
);

GLYPH(glyph_R,
    0b11110,
    0b10001,
    0b10001,
    0b11110,
    0b10100,
    0b10010,
    0b10001
);

GLYPH(glyph_S,
    0b01111,
    0b10000,
    0b10000,
    0b01110,
    0b00001,
    0b00001,
    0b11110
);

GLYPH(glyph_T,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100
);

GLYPH(glyph_U,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b01110
);

GLYPH(glyph_V,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b01010,
    0b00100
);

GLYPH(glyph_W,
    0b10001,
    0b10001,
    0b10001,
    0b10101,
    0b10101,
    0b11011,
    0b10001
);

GLYPH(glyph_X,
    0b10001,
    0b10001,
    0b01010,
    0b00100,
    0b01010,
    0b10001,
    0b10001
);

GLYPH(glyph_Y,
    0b10001,
    0b10001,
    0b01010,
    0b00100,
    0b00100,
    0b00100,
    0b00100
);

GLYPH(glyph_Z,
    0b11111,
    0b00001,
    0b00010,
    0b00100,
    0b01000,
    0b10000,
    0b11111
);

// lowercase

GLYPH(glyph_a,
    0b00000,
    0b00000,
    0b01110,
    0b00001,
    0b01111,
    0b10001,
    0b01111
);

GLYPH(glyph_b,
    0b10000,
    0b10000,
    0b10110,
    0b11001,
    0b10001,
    0b10001,
    0b11110
);

GLYPH(glyph_c,
    0b00000,
    0b00000,
    0b01110,
    0b10001,
    0b10000,
    0b10001,
    0b01110
);

GLYPH(glyph_d,
    0b00001,
    0b00001,
    0b01101,
    0b10011,
    0b10001,
    0b10001,
    0b01111
);

GLYPH(glyph_e,
    0b00000,
    0b00000,
    0b01110,
    0b10001,
    0b11111,
    0b10000,
    0b01111
);

GLYPH(glyph_f,
    0b00110,
    0b01001,
    0b01000,
    0b11100,
    0b01000,
    0b01000,
    0b01000
);

GLYPH(glyph_g,
    0b01101,
    0b10011,
    0b10001,
    0b01111,
    0b00001,
    0b11110,
    0b00000,
);

GLYPH(glyph_h,
    0b10000,
    0b10000,
    0b10110,
    0b11001,
    0b10001,
    0b10001,
    0b10001
);

GLYPH(glyph_i,
    0b00100,
    0b00000,
    0b01100,
    0b00100,
    0b00100,
    0b00100,
    0b01110
);

GLYPH(glyph_j,
    0b00010,
    0b00000,
    0b00110,
    0b00010,
    0b00010,
    0b10010,
    0b01100
);

GLYPH(glyph_k,
    0b10000,
    0b10000,
    0b10010,
    0b10100,
    0b11000,
    0b10100,
    0b10010
);

GLYPH(glyph_l,
    0b01100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b01110
);

GLYPH(glyph_m,
    0b00000,
    0b00000,
    0b11010,
    0b10101,
    0b10101,
    0b10101,
    0b10101
);

GLYPH(glyph_n,
    0b00000,
    0b00000,
    0b10110,
    0b11001,
    0b10001,
    0b10001,
    0b10001
);

GLYPH(glyph_o,
    0b00000,
    0b00000,
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b01110
);

GLYPH(glyph_p,
    0b11110,
    0b10001,
    0b10001,
    0b10001,
    0b11110,
    0b10000,
    0b10000,
);

GLYPH(glyph_q,
    0b01111,
    0b10001,
    0b10001,
    0b10001,
    0b01111,
    0b00001,
    0b00001,
);

GLYPH(glyph_r,
    0b00000,
    0b00000,
    0b10110,
    0b11001,
    0b10000,
    0b10000,
    0b10000
);

GLYPH(glyph_s,
    0b00000,
    0b00000,
    0b01111,
    0b10000,
    0b01110,
    0b00001,
    0b11110
);

GLYPH(glyph_t,
    0b01000,
    0b01000,
    0b11100,
    0b01000,
    0b01000,
    0b01001,
    0b00110
);

GLYPH(glyph_u,
    0b00000,
    0b00000,
    0b10001,
    0b10001,
    0b10001,
    0b10011,
    0b01101
);

GLYPH(glyph_v,
    0b00000,
    0b00000,
    0b10001,
    0b10001,
    0b10001,
    0b01010,
    0b00100
);

GLYPH(glyph_w,
    0b00000,
    0b00000,
    0b10001,
    0b10101,
    0b10101,
    0b10101,
    0b01010
);

GLYPH(glyph_x,
    0b00000,
    0b00000,
    0b10001,
    0b01010,
    0b00100,
    0b01010,
    0b10001
);

GLYPH(glyph_y,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b01111,
    0b00001,
    0b01110
);

GLYPH(glyph_z,
    0b00000,
    0b00000,
    0b11111,
    0b00010,
    0b00100,
    0b01000,
    0b11111
);

GLYPH(glyph_0,
    0b01110,
    0b10001,
    0b10011,
    0b10101,
    0b11001,
    0b10001,
    0b01110
);

GLYPH(glyph_1,
    0b010,
    0b110,
    0b010,
    0b010,
    0b010,
    0b010,
    0b111
);

GLYPH(glyph_2,
    0b01110,
    0b10001,
    0b00001,
    0b01110,
    0b10000,
    0b10000,
    0b11111
);

GLYPH(glyph_3,
    0b1110,
    0b0001,
    0b0001,
    0b0110,
    0b0001,
    0b0001,
    0b1110,
);

GLYPH(glyph_4,
    0b00010,
    0b00110,
    0b01010,
    0b10010,
    0b11111,
    0b00010,
    0b00010,
);

GLYPH(glyph_5,
    0b11111,
    0b10000,
    0b11110,
    0b00001,
    0b00001,
    0b10001,
    0b01110,
);

GLYPH(glyph_6,
    0b00111,
    0b01000,
    0b10000,
    0b11110,
    0b10001,
    0b10001,
    0b01110,
);

GLYPH(glyph_7,
    0b11111,
    0b00001,
    0b00010,
    0b00100,
    0b01000,
    0b10000,
    0b10000,
);

GLYPH(glyph_8,
    0b01110,
    0b10001,
    0b10001,
    0b01110,
    0b10001,
    0b10001,
    0b01110,
);

GLYPH(glyph_9,
    0b01110,
    0b10001,
    0b10001,
    0b01111,
    0b00001,
    0b00010,
    0b11100,
);

GLYPH(glyph_colon,
    0b0,
    0b1,
    0b0,
    0b0,
    0b0,
    0b1,
    0b0,
);

GLYPH(glyph_exclaim,
    0b1,
    0b1,
    0b1,
    0b1,
    0b1,
    0b0,
    0b1,
);

GLYPH(glyph_question,
    0b01110,
    0b10001,
    0b00001,
    0b00111,
    0b00100,
    0b00000,
    0b00100,
);

GLYPH(glyph_semicolon,
    0b00,
    0b01,
    0b00,
    0b00,
    0b00,
    0b01,
    0b10,
);

GLYPH(glyph_dollar,
    0b00100,
    0b01111,
    0b10100,
    0b01110,
    0b00101,
    0b11110,
    0b00100
);

GLYPH(glyph_atsign,
    0b01110,
    0b10001,
    0b10101,
    0b10111,
    0b10110,
    0b10000,
    0b01111
);

GLYPH(glyph_hash,
    0b01010,
    0b01010,
    0b11111,
    0b01010,
    0b11111,
    0b01010,
    0b01010,
);

GLYPH(glyph_ampersand,
    0b01000,
    0b10100,
    0b10100,
    0b01000,
    0b10101,
    0b10010,
    0b01101,
);

GLYPH(glyph_percent,
    0b11000,
    0b11001,
    0b00010,
    0b00100,
    0b01000,
    0b10011,
    0b00011,
);

GLYPH(glyph_caret,
    0b00100,
    0b01010,
    0b10001,
);

GLYPH(glyph_openbrack,
    0b011,
    0b100,
    0b100,
    0b100,
    0b100,
    0b100,
    0b011,
);

GLYPH(glyph_closedbrack,
    0b110,
    0b001,
    0b001,
    0b001,
    0b001,
    0b001,
    0b110,
);

GLYPH(glyph_slash,
    0b0001,
    0b0010,
    0b0010,
    0b0100,
    0b0100,
    0b1000,
    0b1000,
);

GLYPH(glyph_dash,
    0b11111,
);

GLYPH(glyph_tilda,
    0b01000,
    0b10101,
    0b00010,
);

GLYPH(glyph_asterisk,
    0b10101,
    0b01110,
    0b11111,
    0b01110,
    0b10101,
);

GLYPH(glyph_verticalbar,
    0b1,
    0b1,
    0b1,
    0b1,
    0b1,
    0b1,
    0b1,
);

/* glyph template

GLYPH(glyph_X,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
);

*/

const Glyph glyphs[] =
{
    { ' ', 3, 7, 4, 0, 0, glyph_space },

    { 'A', 5, 7, 6, 0, 0, glyph_A },
    { 'B', 5, 7, 6, 0, 0, glyph_B },
    { 'C', 5, 7, 6, 0, 0, glyph_C },
    { 'D', 5, 7, 6, 0, 0, glyph_D },
    { 'E', 5, 7, 6, 0, 0, glyph_E },
    { 'F', 5, 7, 6, 0, 0, glyph_F },
    { 'G', 5, 7, 6, 0, 0, glyph_G },
    { 'H', 5, 7, 6, 0, 0, glyph_H },
    { 'I', 5, 7, 6, 0, 0, glyph_I },
    { 'J', 5, 7, 6, 0, 0, glyph_J },
    { 'K', 5, 7, 6, 0, 0, glyph_K },
    { 'L', 5, 7, 6, 0, 0, glyph_L },
    { 'M', 5, 7, 6, 0, 0, glyph_M },
    { 'N', 5, 7, 6, 0, 0, glyph_N },
    { 'O', 5, 7, 6, 0, 0, glyph_O },
    { 'P', 5, 7, 6, 0, 0, glyph_P },
    { 'Q', 5, 7, 6, 0, 0, glyph_Q },
    { 'R', 5, 7, 6, 0, 0, glyph_R },
    { 'S', 5, 7, 6, 0, 0, glyph_S },
    { 'T', 5, 7, 6, 0, 0, glyph_T },
    { 'U', 5, 7, 6, 0, 0, glyph_U },
    { 'V', 5, 7, 6, 0, 0, glyph_V },
    { 'W', 5, 7, 6, 0, 0, glyph_W },
    { 'X', 5, 7, 6, 0, 0, glyph_X },
    { 'Y', 5, 7, 6, 0, 0, glyph_Y },
    { 'Z', 5, 7, 6, 0, 0, glyph_Z },

    { 'a', 5, 7, 6, 0, 0, glyph_a },
    { 'b', 5, 7, 6, 0, 0, glyph_b },
    { 'c', 5, 7, 6, 0, 0, glyph_c },
    { 'd', 5, 7, 6, 0, 0, glyph_d },
    { 'e', 5, 7, 6, 0, 0, glyph_e },
    { 'f', 5, 7, 6, 0, 0, glyph_f },
    { 'g', 5, 7, 6, 0, 2, glyph_g },
    { 'h', 5, 7, 6, 0, 0, glyph_h },
    { 'i', 4, 7, 4, 0, 0, glyph_i },
    { 'j', 5, 7, 6, 0, 0, glyph_j },
    { 'k', 5, 7, 6, 0, 0, glyph_k },
    { 'l', 5, 7, 6, 0, 0, glyph_l },
    { 'm', 5, 7, 6, 0, 0, glyph_m },
    { 'n', 5, 7, 6, 0, 0, glyph_n },
    { 'o', 5, 7, 6, 0, 0, glyph_o },
    { 'p', 5, 7, 6, 0, 2, glyph_p },
    { 'q', 5, 7, 6, 0, 2, glyph_q },
    { 'r', 5, 7, 6, 0, 0, glyph_r },
    { 's', 5, 7, 6, 0, 0, glyph_s },
    { 't', 5, 7, 6, 0, 0, glyph_t },
    { 'u', 5, 7, 6, 0, 0, glyph_u },
    { 'v', 5, 7, 6, 0, 0, glyph_v },
    { 'w', 5, 7, 6, 0, 0, glyph_w },
    { 'x', 5, 7, 6, 0, 0, glyph_x },
    { 'y', 5, 7, 6, 0, 2, glyph_y },
    { 'z', 5, 7, 6, 0, 0, glyph_z },

    { '0', 5, 7, 6, 0, 0, glyph_0 },
    { '1', 3, 7, 4, 0, 0, glyph_1 },
    { '2', 5, 7, 6, 0, 0, glyph_2 },
    { '3', 4, 7, 5, 0, 0, glyph_3 },
    { '4', 5, 7, 6, 0, 0, glyph_4 },
    { '5', 5, 7, 6, 0, 0, glyph_5 },
    { '6', 5, 7, 6, 0, 0, glyph_6 },
    { '7', 5, 7, 6, 0, 0, glyph_7 },
    { '8', 5, 7, 6, 0, 0, glyph_8 },
    { '9', 5, 7, 6, 0, 0, glyph_9 },

    { ':', 1, 7, 3, 0, 0, glyph_colon },
    { '!', 1, 7, 3, 0, 0, glyph_exclaim },
    { '?', 5, 7, 6, 0, 0, glyph_question },
    { ';', 2, 7, 4, 0, 0, glyph_semicolon },
    { '$', 5, 7, 6, 0, 0, glyph_dollar },
    { '@', 5, 7, 6, 0, 0, glyph_atsign },
    { '#', 5, 7, 6, 0, 0, glyph_hash },
    { '&', 5, 7, 6, 0, 0, glyph_ampersand },
    { '%', 5, 7, 6, 0, 0, glyph_percent },
    { '^', 5, 3, 6, 0, 0, glyph_caret },
    { '(', 3, 7, 4, 0, 0, glyph_openbrack },
    { ')', 3, 7, 4, 0, 0, glyph_closedbrack },
    { '/', 4, 7, 5, 0, 0, glyph_slash },
    { '-', 5, 1, 6, 0, 3, glyph_dash },
    { '_', 5, 1, 6, 0, 6, glyph_dash },
    { '*', 5, 5, 6, 0, 1, glyph_asterisk },
    { '~', 5, 3, 6, 0, 2, glyph_tilda },
    { '|', 1, 7, 2, 0, 0, glyph_verticalbar },
    
};

} // namespace

const Font Font5x7 =
{
    7,
    1,
    glyphs,
    static_cast<uint16_t>(
        sizeof(glyphs) / sizeof(glyphs[0])
    )
};

} // namespace fonts
} // namespace display