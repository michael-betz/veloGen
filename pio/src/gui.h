#ifndef GUI_H
#define GUI_H

#include <string>
#include "lv_font.h"

//-----------------------------------------------
// Dirty GUI stuff
//-----------------------------------------------
// Horizontal alignment
typedef enum {
    LV_LEFT,
    LV_CENTER,
    LV_RIGHT
} t_align;

typedef struct {
    // text origin
    int x;
    int y;
    // clip window
    int x0;
    int y0;
    int x1;
    int y1;
    t_align align;
    lv_font_t *fnt;
} t_label;

// x0, y0 is the anchor point, which is on the top left / middle / right
// depending on the chosen alignment
// size of the bounding box, which is erased for redraws, is inferred from `init` text
void lv_init_label(t_label *lbl, int x0, int y0, int y_shift, lv_font_t *fnt, const char *init, t_align a);
void lv_update_label(t_label *lbl, const std::string &txt);
void lv_print(const std::string &str);

#endif
