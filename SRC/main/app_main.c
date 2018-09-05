// Copyright 2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/sigmadelta.h"
#include "libesphttpd/esp.h"
#include "libesphttpd/cgiwebsocket.h"
#include "esp_log.h"
#include "web_console.h"
#include "wifi_startup.h"
#include "velogen_hw.h"
#include "app_main.h"

static const char *T = "MAIN_APP";

static void wsAppReceive(Websock *ws, char *data, int len, int flags){
    int temp;
    if(len>1){
        switch(data[0]){
            case 'p':
                temp = atoi(&data[1]);
                if(temp >= 0 && temp <= 127){
                    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, temp);
                    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
                }
                break;
            case 'I':
                temp = atoi(&data[1]);
                if(temp >= -128 && temp <= 127){
                    sigmadelta_set_duty(SIGMADELTA_CHANNEL_0, temp);
                }
                break;
            default:
                ESP_LOGI(T, "wsrx:");
                ESP_LOG_BUFFER_HEXDUMP(T, data, len, ESP_LOG_INFO);
        }
    }
}

// Websocket connected.
void wsAppConnect(Websock *ws) {
    uint8_t *rip = ws->conn->remote_ip;
    ESP_LOGI(T,"WS-APP-client: %d.%d.%d.%d", rip[0], rip[1], rip[2], rip[3] );
    ws->recvCb = wsAppReceive;
}

void app_main(){
    gpio_set_level( GPIO_LED, 1 );

    //------------------------------
    // Enable RAM log file
    //------------------------------
    esp_log_level_set( "*", ESP_LOG_INFO );
    esp_log_set_vprintf( wsDebugPrintf );

    //------------------------------
    // Init filesystems
    //------------------------------
    initSpiffs();

    //------------------------------
    // Velogen stuff
    //------------------------------
    if( esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ){
        ESP_LOGI(T, "************* Woke up from the DEEEEAAADD *************");
    } else {
        ESP_LOGI(T, "ZZzzZZZZzzzZZZZzZzz Woke up from a deep nap ZZzzzzZZZzzzzZZZz");
    }
    initVelogen();

    //------------------------------
    // Startup wifi & webserver
    //------------------------------
    ESP_LOGI(T,"Starting network infrastructure ...");
    wifi_conn_init();
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, 0, 0, 20000/portTICK_PERIOD_MS);
    // vTaskDelay(3000 / portTICK_PERIOD_MS);

    //------------------------------
    // Set the clock / print time
    //------------------------------
    // Set timezone to Eastern Standard Time
    setenv("TZ", "PST8PDT", 1);
    tzset();

    xTaskCreate(&adc_monitor_task, "adc_monitor_task", 2048, NULL, 5, NULL);
    ESP_LOGI(T, "Done");
    gpio_set_level( GPIO_LED, 0 );
}
