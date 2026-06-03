#pragma once
#include "driver/gpio.h"

#define E(x) ESP_ERROR_CHECK_WITHOUT_ABORT(x)
#define log_e(format, ...) ESP_LOGE(T, format, ##__VA_ARGS__)
#define log_w(format, ...) ESP_LOGW(T, format, ##__VA_ARGS__)
#define log_i(format, ...) ESP_LOGI(T, format, ##__VA_ARGS__)
#define log_d(format, ...) ESP_LOGD(T, format, ##__VA_ARGS__)
#define log_v(format, ...) ESP_LOGV(T, format, ##__VA_ARGS__)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// GPIO Pin definitions
#define P_BOOT0 GPIO_NUM_0     // The bootloader button
#define P_AUX_PWR GPIO_NUM_16  // Auxiliary power on / off
#define P_AC GPIO_NUM_15       // N zero crossings per rotation
#define P_EN0 GPIO_NUM_27      // Handlebar
#define P_EN1 GPIO_NUM_26      // LED string data
#define P_EN2 GPIO_NUM_36      // Empty
#define P_GPS_TX GPIO_NUM_22   // UBLOX NEO 6M
#define P_GPS_RX GPIO_NUM_23
#define P_GPS_EN GPIO_NUM_21 // PNP controlling GPS power

#define CYCLE_MS 50
#define N_LEDS 6  // 17           // Length of the LED strip

#define F_PREFIX "/lfs"

// I think it's in [bytes] -1 if no update is in progress.
extern int ota_n_written;
