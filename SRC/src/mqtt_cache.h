#ifndef MQTT_CACHE_H
#define MQTT_CACHE_H
#include "mqtt_client.h"

extern FILE *f_buf;

extern esp_mqtt_client_handle_t mqtt_c;
extern bool isMqttConnect;

void cache_init();
void cache_handle();
void mqtt_init();
#endif
