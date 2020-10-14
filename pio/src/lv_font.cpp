// MIT licence
// Copyright (c) 2020 LVGL LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// This is the essence of the LVGL bitmap-font engine.
// Code is mostly verbatim copied from LVGL and stripped down:
//   * no compression (not effective for small sizes anyway)
//   * only 1 bpp

#include <stdio.h>
#include <string>
#include "lv_font.h"
#include "ssd1306.h"

//--------------------
// private variables
//--------------------
static const lv_font_t *cur_font;
static int curs_x=0, curs_x_=0, curs_y=0;


/** Searches base[0] to base[n - 1] for an item that matches *key.
 *
 * @note The function cmp must return negative if its first
 *  argument (the search key) is less that its second (a table entry),
 *  zero if equal, and positive if greater.
 *
 *  @note Items in the array must be in ascending order.
 *
 * @param key    Pointer to item being searched for
 * @param base   Pointer to first element to search
 * @param n      Number of elements
 * @param size   Size of each element
 * @param cmp    Pointer to comparison function (see #lv_font_codeCompare as a comparison function
 * example)
 *
 * @return a pointer to a matching item, or NULL if none exists.
 */
static void * _lv_utils_bsearch(const void * key, const void * base, uint32_t n, uint32_t size,
                         int32_t (*cmp)(const void * pRef, const void * pElement))
{
    const char * middle;
    int32_t c;

    for(middle = (const char *)base; n != 0;) {
        middle += (n / 2) * size;
        if((c = (*cmp)(key, middle)) > 0) {
            n    = (n / 2) - ((n & 1) == 0);
            base = (middle += size);
        }
        else if(c < 0) {
            n /= 2;
            middle = (const char *)base;
        }
        else {
            return (char *)middle;
        }
    }
    return NULL;
}

/** Code Comparator.
 *
 *  Compares the value of both input arguments.
 *
 *  @param[in]  pRef        Pointer to the reference.
 *  @param[in]  pElement    Pointer to the element to compare.
 *
 *  @return Result of comparison.
 *  @retval < 0   Reference is greater than element.
 *  @retval = 0   Reference is equal to element.
 *  @retval > 0   Reference is less than element.
 *
 */
static int32_t unicode_list_compare(const void * ref, const void * element)
{
    return ((int32_t)(*(uint16_t *)ref)) - ((int32_t)(*(uint16_t *)element));
}

static uint32_t get_glyph_dsc_id(const lv_font_t * font, uint32_t letter)
{
    if(letter == '\0') return 0;

    lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;

    /*Check the cache first*/
    if(letter == fdsc->last_letter) return fdsc->last_glyph_id;

    uint16_t i;
    for(i = 0; i < fdsc->cmap_num; i++) {

        /*Relative code point*/
        uint32_t rcp = letter - fdsc->cmaps[i].range_start;
        if(rcp > fdsc->cmaps[i].range_length) continue;
        uint32_t glyph_id = 0;
        if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY) {
            glyph_id = fdsc->cmaps[i].glyph_id_start + rcp;
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL) {
            const uint8_t * gid_ofs_8 = (const uint8_t *)fdsc->cmaps[i].glyph_id_ofs_list;
            glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_8[rcp];
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_TINY) {
            uint16_t key = rcp;
            uint8_t * p = (uint8_t *)_lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                            sizeof(fdsc->cmaps[i].unicode_list[0]), unicode_list_compare);

            if(p) {
                uintptr_t ofs = (uintptr_t)(p - (uint8_t *) fdsc->cmaps[i].unicode_list);
                ofs = ofs >> 1;     /*The list stores `uint16_t` so the get the index divide by 2*/
                glyph_id = fdsc->cmaps[i].glyph_id_start + ofs;
            }
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_FULL) {
            uint16_t key = rcp;
            uint8_t * p = (uint8_t *)_lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                            sizeof(fdsc->cmaps[i].unicode_list[0]), unicode_list_compare);

            if(p) {
                uintptr_t ofs = (uintptr_t)(p - (uint8_t *) fdsc->cmaps[i].unicode_list);
                ofs = ofs >> 1;     /*The list stores `uint16_t` so the get the index divide by 2*/
                const uint8_t * gid_ofs_16 = (const uint8_t *)fdsc->cmaps[i].glyph_id_ofs_list;
                glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_16[ofs];
            }
        }

        /*Update the cache*/
        fdsc->last_letter = letter;
        fdsc->last_glyph_id = glyph_id;
        return glyph_id;
    }

    fdsc->last_letter = letter;
    fdsc->last_glyph_id = 0;
    return 0;
}

/**
 * Used as `get_glyph_bitmap` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font pointer to font
 * @param unicode_letter an unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
const uint8_t * get_glyph_bitmap(const lv_font_t * font, uint32_t unicode_letter)
{
    if(unicode_letter == '\t') unicode_letter = ' ';

    lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;
    uint32_t gid = get_glyph_dsc_id(font, unicode_letter);
    if(!gid) return NULL;

    const lv_font_fmt_txt_glyph_dsc_t * gdsc = &fdsc->glyph_dsc[gid];

    if(fdsc->bitmap_format == LV_FONT_FMT_TXT_PLAIN) {
        if(gdsc) return &fdsc->glyph_bitmap[gdsc->bitmap_index];
    }
    /*Don't Handle compressed bitmap*/
    else {
        return NULL;
    }

    /*If not returned earlier then the letter is not found in this font*/
    return NULL;
}

/**
 * Used as `get_glyph_dsc` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font_p pointer to font
 * @param dsc_out store the result descriptor here
 * @param letter an UNICODE letter code
 * @return true: descriptor is successfully loaded into `dsc_out`.
 *         false: the letter was not found, no data is loaded to `dsc_out`
 */
bool get_glyph_dsc(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter)
{
    bool is_tab = false;
    if(unicode_letter == '\t') {
        unicode_letter = ' ';
        is_tab = true;
    }
    lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;
    uint32_t gid = get_glyph_dsc_id(font, unicode_letter);
    if(!gid) return false;

    /*Put together a glyph dsc*/
    const lv_font_fmt_txt_glyph_dsc_t * gdsc = &fdsc->glyph_dsc[gid];

    // int32_t kv = ((int32_t)((int32_t)kvalue * fdsc->kern_scale) >> 4);

    uint32_t adv_w = gdsc->adv_w;
    if(is_tab) adv_w *= 2;

    // adv_w += kv;
    adv_w  = (adv_w + (1 << 3)) >> 4;

    dsc_out->adv_w = adv_w;
    dsc_out->box_h = gdsc->box_h;
    dsc_out->box_w = gdsc->box_w;
    dsc_out->ofs_x = gdsc->ofs_x;
    dsc_out->ofs_y = gdsc->ofs_y;
    dsc_out->bpp   = (uint8_t)fdsc->bpp;

    if(is_tab) dsc_out->box_w = dsc_out->box_w * 2;

    return true;
}

// decodes one UTF8 character at a time, keeping internal state
// returns a valid unicode character once complete or 0
//
// Unicode return value                   UTF-8 encoded chars
// 00000000 00000000 00000000 0xxxxxxx -> 0xxxxxxx
// 00000000 00000000 00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx
// 00000000 00000000 zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx
// 00000000 000wwwzz zzzzyyyy yyxxxxxx -> 11110www 10zzzzzz 10yyyyyy 10xxxxxx
static uint32_t utf8_dec(char c)
{
    static unsigned readN=0, result=0;

    if ((c & 0x80) == 0) {
        // 1 byte character, nothing to decode
        readN = 0;  // reset state
        return c;
    }

    if (readN == 0) {
        // first byte of several, initialize N bytes decode
        if ((c & 0xE0) == 0xC0)  // 2 bytes to decode
            readN = 2;
        else if ((c & 0xF0) == 0xE0)  // 3 bytes to decode
            readN = 3;
        else if ((c & 0xF8) == 0xF0)  // 4 bytes to decode
            readN = 4;
        result = 0;
    }

    switch (readN) {
        case 1:
            result |= c & 0x3F;
            readN = 0;
            return result;

        case 2:
            result |= (c & 0x1F) << 6;
            break;

        case 3:
            result |= (c & 0x0F) << 12;
            break;

        case 4:
            result |= (c & 0x07) << 18;
            break;

        default:
            readN = 1;
    }
    readN--;

    return 0;
}

#define LV_BPP_MASK ((1 << LV_BPP) - 1)
static int bb_x0=0, bb_x1=DISPLAY_WIDTH, bb_y0=0, bb_y1=DISPLAY_HEIGHT;

static bool check_bb(int x, int y)
{
    return (x >= bb_x0 && x <= bb_x1 && y >= bb_y0 && y <= bb_y1);
}

static void draw_glyph(const uint8_t *bmp, unsigned x, unsigned y, unsigned w, unsigned h)
{
    int nBitsLoaded = 0;
    unsigned bits = 0;

    y -= h;

    for (unsigned row=0; row<h; row++) {
        for (unsigned col=0; col<w; col++) {
            if (nBitsLoaded < LV_BPP) {
                // fill up 8 bits from the right (LSBs)
                bits <<= 8;
                bits |= *bmp++;
                nBitsLoaded += 8;
            }
            // consume LV_BPP bits from the left (MSBs)
            unsigned tmp = (bits >> (nBitsLoaded - LV_BPP)) & LV_BPP_MASK;
            if (tmp && check_bb(col + x, row + y))
                setPixel(col + x, row + y, 1);
            nBitsLoaded -= LV_BPP;
        }
    }
}

// needs to be called before drawing any characters!
void set_font(lv_font_t *f)
{
    if (f)
        cur_font = f;
}

// set upper left position for draw_char()
void set_cursor(int x, int y)
{
    curs_x = x;
    curs_x_ = x;
    curs_y = y;
}

// set upper left position for draw_char()
void set_bb(int x0, int x1, int y0, int y1)
{
    bb_x0 = x0;
    bb_x1 = x1;
    bb_y0 = y0;
    bb_y1 = y1;
}

void reset_bb()
{
    bb_x0=0;
    bb_x1=DISPLAY_WIDTH;
    bb_y0=0;
    bb_y1=DISPLAY_HEIGHT;
}

// get bounding box (width and height) of the rendered text
void get_bb(const char *txt, int *w, int *h)
{
    unsigned dc=0;
    int x=0;
    lv_font_glyph_dsc_t g;
    while (*txt) {
        dc = utf8_dec(*txt++);
        if (dc == 0)
            continue;
        get_glyph_dsc(cur_font, &g, dc);
        x += g.adv_w;
    }
    if (w)
        *w = x;
    if (h)
        *h = cur_font->line_height; // - cur_font->base_line;
}

// call this in picorv32s _putchar() to use it with the `print_*()` functions
void draw_char(char c)
{
    lv_font_glyph_dsc_t g;
    unsigned dc = utf8_dec(c);
    if (dc == 0)
        return;

    // These don't really work very well. Too bad.
    if (dc == '\n' || dc == '\r')
        curs_x = curs_x_;
    if (dc == '\n') {
        curs_y += cur_font->line_height;
        return;
    }

    unsigned y = cur_font->line_height - cur_font->base_line + curs_y;

    get_glyph_dsc(cur_font, &g, dc);
    if (g.box_h > 0 && g.box_w > 0) {
        const uint8_t *bmp = get_glyph_bitmap(cur_font, dc);
        if (bmp)
            draw_glyph(bmp, curs_x + g.ofs_x, y - g.ofs_y, g.box_w, g.box_h);
    }
    curs_x += g.adv_w;
}
