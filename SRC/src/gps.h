#pragma once
#include "nmea_parser.h"

extern gps_t g_gps_data;

void gps_init_parser();
void gps_deinit_parser();
void gps_wake();
void gps_sleep();

// To let ustudio connect to UART over TCP
void gps_proxy_start(void);
void gps_proxy_stop(void);

// high level init depending on .json settings
void gps_init_from_json();
