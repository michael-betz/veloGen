#ifndef WIFI_STARTUP_H
#define WIFI_STARTUP_H

#include "freertos/event_groups.h"

#define CONNECTED_BIT BIT0
extern EventGroupHandle_t wifi_event_group;

extern void wifi_conn_init(void);

#endif