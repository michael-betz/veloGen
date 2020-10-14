#include <string>
#include <sstream>
#include "Arduino.h"
#include "gui.h"
#include "lv_font.h"
#include "ssd1306.h"
#include "ina219.h"

extern lv_font_t noto_sans_12;
extern lv_font_t concert_one_50;

// screen layout: big number + unit on top left
void big_num(bool isInit, int type)
{
    static t_label big_lbl;
    static const char * const units[] = {"km/h", "mA", "mV", "mW"};
    if (isInit) {
        // Static content which will not be refreshed
        lv_init_label(&big_lbl, 127, 0, 0, &noto_sans_12, units[type], LV_RIGHT);

        // Setup the bounding box of dynamic labels
        lv_init_label(&big_lbl, 64, 18, 0, &concert_one_50, "22222", LV_CENTER);
    }

    static int tmp_kmh = 0;
    std::ostringstream bla;

    int volts = inaV();
    int amps = inaI();

    // clears and prints dynamic content into BB
    switch (type) {
        case 0:
            bla << tmp_kmh / 10 << "." << tmp_kmh % 10;
            break;

        case 1:
            bla << amps;
            break;

        case 2:
            bla << volts;
            break;

        case 3:
            bla << volts * amps / 1000;
            break;
    }

    lv_update_label(&big_lbl, bla.str());
    tmp_kmh++;
}

// -1 = just update, don't change
void draw_screen(int screen_id)
{
    static int cur_screen_id = -1;
    bool isInit = false;
    if (screen_id >= 0 && screen_id != cur_screen_id) {
        isInit = true;
        fill(0);
        // draw_status();
        cur_screen_id = screen_id;
    }

    switch (cur_screen_id) {
        case 0:
        case 1:
        case 2:
        case 3:
            big_num(isInit, cur_screen_id);
            break;
    }
    ssd_send();
}
