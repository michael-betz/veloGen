// #include <string>
#include "Arduino.h"
#include "gui.h"
#include "lv_font.h"
#include "ssd1306.h"
#include "ina219.h"
#include "velogen.h"

extern lv_font_t noto_sans_12;
extern lv_font_t concert_one_50;

#define N_SCREENS 4

// screen layout: big number + unit on top left
static void big_num(bool isInit, int type, unsigned btns)
{
    char buff[32];
    static t_label big_lbl;
    static const char * const units[] = {"km/h", "mA", "mV", "mW"};

    if (isInit) {
        // Static content which will not be refreshed
        lv_init_label(&big_lbl, 127, 0, 0, &noto_sans_12, units[type], LV_RIGHT);

        // Setup the bounding box of dynamic labels
        lv_init_label(&big_lbl, 64, 18, 0, &concert_one_50, "22222", LV_CENTER);
    }

    int volts = inaV();
    int amps = inaI();

    // clears and prints dynamic content into BB
    switch (type) {
        case 0:
            snprintf(buff, sizeof(buff), "%.1f", g_speed);
            break;

        case 1:
            snprintf(buff, sizeof(buff), "%d", amps);
            break;

        case 2:
            snprintf(buff, sizeof(buff), "%d", volts);
            break;

        case 3:
            snprintf(buff, sizeof(buff), "%d", volts * amps / 1000);
            break;
    }

    lv_update_label(&big_lbl, buff);
}

// called in the main loop, reads buttons,
// chooses which screen to initialize / refresh
void draw_screen()
{
    static int scr_id = 0;
    static bool isInit = true;

    // corresponding bit is 1 on button released
    unsigned btns = touch_read();

    // left button: switch screen
    if (btns & (1 << 0)) {
        scr_id--;
        if (scr_id < 0)
            scr_id = 0;
    }

    // toggle dynamo
    if (btns & (1 << 1))
        digitalWrite(P_DYN, !digitalRead(P_DYN));

    // right button: switch screen
    if (btns & (1 << 3)) {
        scr_id++;
        if (scr_id >= N_SCREENS)
            scr_id = N_SCREENS - 1;
    }

    // check if screen needs to be re-drawn completely
    if (btns & 0x09) {
        isInit = true;
        fill(0);
        // draw_status();
    }

    switch (scr_id) {
        case 0:
        case 1:
        case 2:
        case 3:
            big_num(isInit, scr_id, btns);
            break;
    }
    ssd_send();
    isInit = false;
}
