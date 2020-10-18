#include "lv_font.h"

/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --size 12 --bpp 1 --format lvgl --font fonts/NotoSans-Regular.ttf -r 0x20-0x7F --no-kerning -o src/noto_sans_12.c
 ******************************************************************************/

#ifndef NOTO_SANS_12
#define NOTO_SANS_12 1
#endif

#if NOTO_SANS_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */
    0x0,

    /* U+21 "!" */
    0xfd, 0x80,

    /* U+22 "\"" */
    0x0, 0xd, 0x59,

    /* U+23 "#" */
    0x14, 0x29, 0xf9, 0x22, 0x5f, 0xca, 0x24,

    /* U+24 "$" */
    0x23, 0xe9, 0x4e, 0x1c, 0xa5, 0xf1, 0x8,

    /* U+25 "%" */
    0x62, 0x4a, 0x25, 0xd, 0x60, 0xc8, 0xa4, 0x52,
    0x46,

    /* U+26 "&" */
    0x30, 0x48, 0x48, 0x58, 0x30, 0xd2, 0x8a, 0x86,
    0x7b,

    /* U+27 "'" */
    0x3, 0x60,

    /* U+28 "(" */
    0x9, 0x29, 0x24, 0x92, 0x24, 0x80,

    /* U+29 ")" */
    0x9, 0x22, 0x49, 0x24, 0xa4, 0x80,

    /* U+2A "*" */
    0x0, 0x2, 0x8, 0xf8, 0xc5, 0x10,

    /* U+2B "+" */
    0x1, 0x8, 0x4f, 0x90, 0x80,

    /* U+2C "," */
    0x58,

    /* U+2D "-" */
    0x0, 0xe,

    /* U+2E "." */
    0xc0,

    /* U+2F "/" */
    0x0, 0x44, 0x21, 0x8, 0x84, 0x21, 0x10, 0x84,
    0x20, 0x0,

    /* U+30 "0" */
    0x76, 0xe3, 0x18, 0xc6, 0x3b, 0x70,

    /* U+31 "1" */
    0x3e, 0x92, 0x49, 0x20,

    /* U+32 "2" */
    0x74, 0x42, 0x11, 0x19, 0x88, 0xf8,

    /* U+33 "3" */
    0x74, 0x42, 0x17, 0x4, 0x21, 0xf0,

    /* U+34 "4" */
    0x8, 0x62, 0x8a, 0x4a, 0x2f, 0xc2, 0x8,

    /* U+35 "5" */
    0xf4, 0x21, 0xe1, 0x84, 0x23, 0xf0,

    /* U+36 "6" */
    0x32, 0x21, 0x6c, 0xc6, 0x39, 0x70,

    /* U+37 "7" */
    0xfc, 0x10, 0x82, 0x10, 0x43, 0x8, 0x60,

    /* U+38 "8" */
    0x74, 0x63, 0xa7, 0x46, 0x31, 0x70,

    /* U+39 "9" */
    0x74, 0x63, 0x17, 0x84, 0x22, 0x60,

    /* U+3A ":" */
    0x30, 0xa,

    /* U+3B ";" */
    0x30, 0xa, 0x80,

    /* U+3C "<" */
    0x0, 0x33, 0x30, 0xc0, 0xc0, 0xc0,

    /* U+3D "=" */
    0x0, 0x3e, 0x0, 0x7c,

    /* U+3E ">" */
    0x2, 0x6, 0x6, 0x19, 0x88, 0x0,

    /* U+3F "?" */
    0xe1, 0x13, 0x64, 0x4, 0x40,

    /* U+40 "@" */
    0x1e, 0x30, 0x97, 0xf6, 0x5a, 0x2d, 0x16, 0x8b,
    0x3a, 0x60, 0x1f, 0x0,

    /* U+41 "A" */
    0x18, 0x18, 0x28, 0x24, 0x24, 0x7c, 0x42, 0x42,
    0x82,

    /* U+42 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+43 "C" */
    0x3d, 0x8, 0x20, 0x82, 0x8, 0x10, 0x3c,

    /* U+44 "D" */
    0xf9, 0xa, 0xc, 0x18, 0x30, 0x60, 0xc2, 0xf8,

    /* U+45 "E" */
    0xfc, 0x21, 0xf, 0x42, 0x10, 0xf8,

    /* U+46 "F" */
    0xfc, 0x21, 0xf, 0x42, 0x10, 0x80,

    /* U+47 "G" */
    0x3e, 0x82, 0x4, 0x8, 0xf0, 0x60, 0xa1, 0x3e,

    /* U+48 "H" */
    0x83, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+49 "I" */
    0xe9, 0x24, 0x92, 0xe0,

    /* U+4A "J" */
    0x24, 0x92, 0x49, 0x24, 0xe0,

    /* U+4B "K" */
    0x8e, 0x69, 0x28, 0xe2, 0xc9, 0x22, 0x84,

    /* U+4C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf8,

    /* U+4D "M" */
    0xc1, 0xe0, 0xe8, 0xb4, 0x5a, 0x2c, 0xa6, 0x53,
    0x39, 0x88, 0x80,

    /* U+4E "N" */
    0xc3, 0x86, 0x8d, 0x19, 0x31, 0x62, 0xc3, 0x86,

    /* U+4F "O" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42,
    0x3c,

    /* U+50 "P" */
    0xf4, 0x63, 0x1f, 0x42, 0x10, 0x80,

    /* U+51 "Q" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42,
    0x3c, 0x4, 0x2,

    /* U+52 "R" */
    0xf2, 0x28, 0xa2, 0xf2, 0x49, 0x22, 0x88,

    /* U+53 "S" */
    0x7c, 0x21, 0x87, 0xc, 0x21, 0xf0,

    /* U+54 "T" */
    0xfc, 0x41, 0x4, 0x10, 0x41, 0x4, 0x10,

    /* U+55 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7c,

    /* U+56 "V" */
    0x82, 0x85, 0x12, 0x22, 0x45, 0xa, 0xc, 0x10,

    /* U+57 "W" */
    0x84, 0x28, 0xcd, 0x29, 0x25, 0x24, 0xa4, 0x62,
    0x8c, 0x61, 0x8c, 0x31, 0x80,

    /* U+58 "X" */
    0x44, 0x88, 0xa1, 0xc1, 0x5, 0xa, 0x22, 0xc6,

    /* U+59 "Y" */
    0xc6, 0x89, 0xb1, 0x43, 0x82, 0x4, 0x8, 0x10,

    /* U+5A "Z" */
    0xfc, 0x30, 0x84, 0x30, 0x84, 0x30, 0xfc,

    /* U+5B "[" */
    0x3a, 0xaa, 0xaa, 0xb0,

    /* U+5C "\\" */
    0x2, 0x10, 0x84, 0x10, 0x84, 0x20, 0x84, 0x21,
    0x4, 0x0,

    /* U+5D "]" */
    0x35, 0x55, 0x55, 0x70,

    /* U+5E "^" */
    0x10, 0xc2, 0x92, 0x45, 0x10,

    /* U+5F "_" */
    0xf8,

    /* U+60 "`" */
    0x48,

    /* U+61 "a" */
    0x70, 0x42, 0xf8, 0xc5, 0xe0,

    /* U+62 "b" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0xce, 0xe0,

    /* U+63 "c" */
    0x7e, 0x21, 0x8, 0x61, 0xe0,

    /* U+64 "d" */
    0x4, 0x10, 0x5d, 0xce, 0x18, 0x61, 0xcd, 0xd0,

    /* U+65 "e" */
    0x76, 0x63, 0xf8, 0x41, 0xe0,

    /* U+66 "f" */
    0x34, 0x4f, 0x44, 0x44, 0x44,

    /* U+67 "g" */
    0x77, 0x38, 0x61, 0x87, 0x37, 0x41, 0x8f, 0xe0,

    /* U+68 "h" */
    0x84, 0x21, 0xec, 0xc6, 0x31, 0x8c, 0x40,

    /* U+69 "i" */
    0x9f, 0xc0,

    /* U+6A "j" */
    0x20, 0x12, 0x49, 0x24, 0x9e,

    /* U+6B "k" */
    0x84, 0x21, 0x39, 0x53, 0x96, 0x94, 0x40,

    /* U+6C "l" */
    0xff, 0xc0,

    /* U+6D "m" */
    0xb7, 0x66, 0x62, 0x31, 0x18, 0x8c, 0x46, 0x22,

    /* U+6E "n" */
    0xb6, 0x63, 0x18, 0xc6, 0x20,

    /* U+6F "o" */
    0x7b, 0x38, 0x61, 0x87, 0x37, 0x80,

    /* U+70 "p" */
    0xbb, 0x38, 0x61, 0x87, 0x3b, 0xa0, 0x82, 0x0,

    /* U+71 "q" */
    0x77, 0x38, 0x61, 0x87, 0x37, 0x41, 0x4, 0x10,

    /* U+72 "r" */
    0xbc, 0x88, 0x88, 0x80,

    /* U+73 "s" */
    0x7c, 0x20, 0xe0, 0x87, 0xc0,

    /* U+74 "t" */
    0x44, 0xf4, 0x44, 0x44, 0x70,

    /* U+75 "u" */
    0x8c, 0x63, 0x18, 0xcd, 0xe0,

    /* U+76 "v" */
    0x85, 0x34, 0x92, 0x38, 0xc3, 0x0,

    /* U+77 "w" */
    0x88, 0xa6, 0x55, 0x2a, 0xa5, 0x32, 0x98, 0x8c,

    /* U+78 "x" */
    0x45, 0x23, 0xc, 0x31, 0x24, 0x40,

    /* U+79 "y" */
    0x85, 0x34, 0x92, 0x38, 0xc3, 0x8, 0x23, 0x0,

    /* U+7A "z" */
    0xf8, 0x8c, 0x44, 0x63, 0xe0,

    /* U+7B "{" */
    0x5, 0x24, 0x94, 0x49, 0x24, 0x40,

    /* U+7C "|" */
    0x15, 0x55, 0x55, 0x50,

    /* U+7D "}" */
    0x11, 0x24, 0x91, 0x49, 0x25, 0x0,

    /* U+7E "~" */
    0x0, 0x1, 0xc3, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 50, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 74, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 6, .adv_w = 125, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 110, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 20, .adv_w = 159, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 141, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 63, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 40, .adv_w = 68, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 46, .adv_w = 68, .box_w = 3, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 52, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 58, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 63, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 64, .adv_w = 65, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 66, .adv_w = 51, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 82, .box_w = 5, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 77, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 106, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 106, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 106, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 131, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 57, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 57, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 142, .adv_w = 106, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 148, .adv_w = 106, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 152, .adv_w = 106, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 158, .adv_w = 99, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 173, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 175, .adv_w = 123, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 121, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 107, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 100, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 142, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 65, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 52, .box_w = 3, .box_h = 12, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 243, .adv_w = 119, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 101, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 256, .adv_w = 174, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 146, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 150, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 116, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 150, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 301, .adv_w = 119, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 105, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 107, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 140, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 179, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 109, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 366, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 68, .box_w = 2, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 377, .adv_w = 82, .box_w = 5, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 387, .adv_w = 68, .box_w = 2, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 391, .adv_w = 106, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 396, .adv_w = 79, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 397, .adv_w = 111, .box_w = 3, .box_h = 2, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 398, .adv_w = 108, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 118, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 92, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 118, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 108, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 66, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 118, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 442, .adv_w = 119, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 50, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 451, .adv_w = 50, .box_w = 3, .box_h = 13, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 456, .adv_w = 103, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 50, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 180, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 119, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 116, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 118, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 492, .adv_w = 118, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 500, .adv_w = 79, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 504, .adv_w = 92, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 69, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 514, .adv_w = 119, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 519, .adv_w = 98, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 525, .adv_w = 151, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 102, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 98, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 547, .adv_w = 90, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 75, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 558, .adv_w = 104, .box_w = 2, .box_h = 15, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 562, .adv_w = 72, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 568, .adv_w = 106, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t noto_sans_12 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};

#endif /*#if NOTO_SANS_12*/

