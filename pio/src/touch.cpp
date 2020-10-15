
#include "Arduino.h"

#define N_PINS 4
#define THRESHOLD 12

// touch pin numbers
static const touch_pad_t tPins[N_PINS] = {
    TOUCH_PAD_NUM1, TOUCH_PAD_NUM2, TOUCH_PAD_NUM0, TOUCH_PAD_NUM9
};

// initial reading at boot (without touch)
static uint16_t tinit[N_PINS];

// Current delta values
static int tpv[N_PINS];

void touch_init()
{
    // Arduino API is broken, use IDF one
    touch_pad_init();
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V);
    touch_pad_set_meas_time(0xF000, 0x2000);
    for (int i=0; i<N_PINS; i++) {
        touch_pad_config(tPins[i], 0);
        touch_pad_read(tPins[i], &tinit[i]);
    }
}

// call in main loop, if a button is released, sets its bit in return value
unsigned touch_read()
{
    static unsigned state_ = 0;
    unsigned state=0, release=0;

    for (int i=0; i<N_PINS; i++) {
        uint16_t tmp;
        touch_pad_read(tPins[i], &tmp);
        tpv[i] = tinit[i] - tmp;
        if (tpv[i] > THRESHOLD) {
            state |= 1 << i;
        } else if ((state_ >> i) & 1) {
            release |= 1 << i;
        }
    }

    // if (state)
    //     log_i("TP: %3d %3d %3d %3d", tpv[0], tpv[1], tpv[2], tpv[3]);

    state_ = state;

    return release;
}
