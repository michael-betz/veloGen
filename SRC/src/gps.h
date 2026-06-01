#pragma once
#include "nmea_parser.h"

extern gps_t g_gps_data;

void gps_init();
void gps_wake();
void gps_sleep();
