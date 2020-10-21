#include <stdarg.h>
#include <stdio.h>
#include "lv_font.h"
#include "ssd1306.h"
#include "gui.h"

void lv_print(const char *str)
{
    reset_bb();
    while (*str)
        draw_char(*str++);
}

// GUI label stuff
// a is text alignment __AND__ anchor point!
void lv_init_label(t_label *lbl, int x, int y, int y_shift, lv_font_t *fnt, const char *init, t_align a)
{
    int w=0, h=0;
    lbl->x = x;
    lbl->y = y - y_shift;
    lbl->y0 = y;
    lbl->fnt = fnt;
    lbl->align = a;
    set_font(fnt);
    get_bb(init, &w, &h);
    if (a == LV_LEFT){
        lbl->x0 = x;
        lbl->x1 = x + w;
    } else if (a == LV_CENTER) {
        lbl->x0 = x - w / 2;
        lbl->x1 = x + w / 2;
    } else if (a == LV_RIGHT) {
        lbl->x0 = x - w;
        lbl->x1 = x;
    }
    lbl->y1 = y + h - y_shift;
    lv_update_label(lbl, init);
}

// call it like printf
void lv_update_label(t_label *lbl, const char *format, ...)
{
    int w=0, h=0;
    char buf[32], *p=buf;

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(buf, sizeof(buf), format, argptr);
    va_end(argptr);

    fillRect(lbl->x0, lbl->x1, lbl->y0, lbl->y1, false);
    // rect(lbl->x0, lbl->x1, lbl->y0, lbl->y1, true);  // show bb

    set_font(lbl->fnt);
    if (lbl->align == LV_LEFT) {
        set_cursor(lbl->x, lbl->y);
    } else if (lbl->align == LV_CENTER) {
        get_bb(buf, &w, &h);
        set_cursor(lbl->x - w / 2, lbl->y);
    } else if (lbl->align == LV_RIGHT) {
        get_bb(buf, &w, &h);
        set_cursor(lbl->x - w, lbl->y);
    }
    set_bb(lbl->x0, lbl->x1, lbl->y0, lbl->y1);
    while (*p)
        draw_char(*p++);
}
