/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "app_main.h"
#include "wifi_startup.h"

static const char *T = "VELORECOVER";

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    wifi_conn_init();

    //------------------------------
    // Setup misc GPIO
    //------------------------------
    gpio_pad_select_gpio( GPIO_LED );
    ESP_ERROR_CHECK( gpio_set_direction( GPIO_LED, GPIO_MODE_OUTPUT ) );

    ESP_LOGI(T, "Hello world!");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(T, "This is ESP32 chip with %d CPU cores, WiFi%s%s, silicon revision %d",
         chip_info.cores,
        (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
         chip_info.revision );
}

// Print a pretty hex-dump on the debug out
void hexDump( uint8_t *buffer, uint16_t nBytes ){
    for( uint16_t i=0; i<nBytes; i++ ){
        if( (nBytes>16) && ((i%16)==0) ){
            printf("\n    %04x: ", i);
        }
        printf("%02x ", *buffer++);
    }
    printf("\n");
}
