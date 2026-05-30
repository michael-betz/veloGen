// Velogen specific hardware
#ifndef VELO_H
#define VELO_H

// ESP_IDF is too verbose
#include "esp_log.h"
#include <stdbool.h>

extern unsigned g_wheelCnt;  // accumulated wheel pulses since power up
extern int g_speed;          // current speed [km * 10 / h]
extern int g_mVolts;         // battery voltage [mV]
extern int g_mAmps;          // battery current, [mA] negative = discharging

// initalize velogen hardware
void velogen_init();
void velogen_loop();
void velogen_sleep(bool isReboot);

// call in main loop, if a button is released, sets the corresponding bit
// in return value
unsigned button_read();

// accumulate wheel rotations in g_wheelCnt
unsigned counter_read();

// 0: Off, 1: On
void setAuxPower(bool val);

#endif
