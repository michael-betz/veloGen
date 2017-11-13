#ifndef WIFI_STARTUP_H
#define WIFI_STARTUP_H

#include "freertos/event_groups.h"

#define N_WIFI_TRYS		3
#define HOSTNAME 		"veloGen"

#define CONNECTED_BIT 	BIT0
#define STA_START_BIT	BIT1

extern EventGroupHandle_t wifi_event_group;

extern void wifi_conn_init(void);
extern void wifi_disable();

#endif