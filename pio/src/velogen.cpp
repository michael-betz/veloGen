#include "Arduino.h"
#include "driver/pcnt.h"
#include "Wire.h"
#include "ssd1306.h"
#include "ina219.h"
#include "velogen.h"
// #include "lv_font.h"
// #include "velogen_gui.h"


#define N_PINS 4
#define THRESHOLD 12

// Number of wheel rotations since power up
RTC_DATA_ATTR unsigned g_wheelCnt;

// touch pin numbers
static const touch_pad_t tPins[N_PINS] = {
    TOUCH_PAD_NUM1, TOUCH_PAD_NUM2, TOUCH_PAD_NUM0, TOUCH_PAD_NUM9
};

// initial reading at boot (without touch)
static uint16_t tinit[N_PINS];

// Current delta values
static int tpv[N_PINS];


// Pulse counter to count wheel rotations
static void counter_init()
{
    // Prepare configuration for the PCNT unit
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = P_AC,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP,  // Keep the primary counter mode if low
        .hctrl_mode = PCNT_MODE_KEEP,  // Keep the primary counter mode if high
        // What to do on the positive or negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,  // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,  // Keep the counter value on the negative edge
        .counter_h_lim = 0x7FFF,
        .counter_l_lim = 0,
        .unit = PCNT_UNIT_0,
        .channel = PCNT_CHANNEL_0,
    };
    // Initialize PCNT unit
    pcnt_unit_config(&pcnt_config);
    gpio_set_pull_mode(P_AC, GPIO_FLOATING);
    // Configure and enable the input filter
    pcnt_set_filter_value(PCNT_UNIT_0, 500);
    pcnt_filter_enable(PCNT_UNIT_0);
    // Initialize PCNT's counter
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    // Everything is set up, now go to counting
    pcnt_counter_resume(PCNT_UNIT_0);
}

float g_speed = 0;

int counter_read()
{
    int diffCnt = 0;
    static int16_t lastCnt = 0;
    static unsigned ts_ = 0;
    unsigned ts = millis();
    int16_t pCnt = 0;
    if(pcnt_get_counter_value(PCNT_UNIT_0, &pCnt) == ESP_OK) {
        diffCnt = pCnt - lastCnt;
        g_wheelCnt += diffCnt;
        lastCnt = pCnt;
    }

    // TODO convert this calculation and IIR filter to fixed point
    // [milli counts / milli second] = [counts / second]
    float dC_dT = diffCnt * 1000.0 / (float)(ts - ts_);
    dC_dT = dC_dT * WHEEL_C / (float)WHEEL_POLES * 60.0 * 60.0 / 1000.0 / 1000.0;  // [km / hour]
    g_speed += 0.05 * (dC_dT - g_speed);

    ts_ = ts;
    return diffCnt;
}

static void touch_init()
{
    // Arduino API is broken, use IDF one
    touch_pad_init();
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V5);
    touch_pad_set_meas_time(0x3000, 0x3000);
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
    //     log_v("TP: %3d %3d %3d %3d", tpv[0], tpv[1], tpv[2], tpv[3]);

    state_ = state;

    return release;
}

void velogen_init()
{
    pinMode(P_DYN, OUTPUT);
    pinMode(P_AC, INPUT);
    digitalWrite(P_DYN, 0);

    counter_init();

    // init oled
    Wire.begin(12, 14, 800000);
    ssd_init();
    ssd_contrast(0);

    // init shunt
    inaInit();
    inaBus32(false);
    inaPga(0);
    inaAvg(7);

    touch_init();
}
