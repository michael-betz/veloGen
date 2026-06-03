#pragma once
#include "mqtt_client.h"
#include <stdio.h>

// Telemetry records are appended here if offline. Close this file before sleep!
extern FILE *record_file;
extern SemaphoreHandle_t telemetry_mutex;

void cache_init();
void cache_handle();
void mqtt_reconnect();
