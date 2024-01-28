// Velogen specific hardware
#ifndef VELO_H
#define VELO_H

// ESP_IDF is too verbose
#include "esp_log.h"
#include <stdbool.h>
#define E(x) ESP_ERROR_CHECK_WITHOUT_ABORT(x)
#define log_e(format, ...) ESP_LOGE(T, format, ##__VA_ARGS__)
#define log_w(format, ...) ESP_LOGW(T, format, ##__VA_ARGS__)
#define log_i(format, ...) ESP_LOGI(T, format, ##__VA_ARGS__)
#define log_d(format, ...) ESP_LOGD(T, format, ##__VA_ARGS__)
#define log_v(format, ...) ESP_LOGV(T, format, ##__VA_ARGS__)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// GPIO Pin definitions
#define P_DYN GPIO_NUM_16  // Dynamo on / off
#define P_AC GPIO_NUM_15  // N zero crossings per rotation
#define P_5V GPIO_NUM_17  // 5V enable
#define P_EN0 GPIO_NUM_27  // LED string data
#define P_EN1 GPIO_NUM_26  // Empty
#define P_EN2 GPIO_NUM_36  // Empty

#define CYCLE_MS 50
#define N_LEDS 6  // 17           // Length of the LED strip

extern unsigned g_wheelCnt;  // accumulated wheel pulses since power up
extern int g_speed;  // current speed [km * 10 / h]
extern int g_mVolts;  // battery voltage [mV]
extern int g_mAmps;  // battery current, [mA] negative = discharging

// initalize velogen hardware
void velogen_init();
void velogen_loop();
void velogen_sleep(bool isReboot);

// call in main loop, if a button is released, sets the corresponding bit
// in return value
unsigned button_read();

// accumulate wheel rotations in g_wheelCnt
unsigned counter_read();

// val: -1: toggle, 0: Off, 1: On
void setDynamo(int val);

#endif
