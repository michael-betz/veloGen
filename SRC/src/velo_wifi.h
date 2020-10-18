#ifndef VELO_WIFI_H
#define VELO_WIFI_H
#include "mqtt_client.h"

extern esp_mqtt_client_handle_t mqtt_c;
extern bool isConnect;

void initVeloWifi();
void tryConnect();

void toggle_wifi();

#endif
