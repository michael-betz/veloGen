#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt.h"
#include "my_mqtt.h"

#include "driver/ledc.h"
#include "driver/sigmadelta.h"
#include "driver/adc.h"

#include "app_main.h"

static const char *T = "VELOGEN";

void init()
{
    nvs_flash_init();
    wifi_conn_init();
    //------------------------------
    // Setup the PWM
    //------------------------------
    ledc_timer_config_t ledc_timer = {
        .bit_num = 7,
        .freq_hz = 400000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
        .duty = STARTUP_DUTY_VALUE,
        .gpio_num = GPIO_SEPIC_PWM,
        .intr_type = LEDC_INTR_DISABLE,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel);
    //------------------------------
    // Setup the DAC
    //------------------------------
    sigmadelta_config_t sigmadelta_cfg = {
       .channel = SIGMADELTA_CHANNEL_0,
       .sigmadelta_duty = STARTUP_DAC_VALUE,
       .sigmadelta_prescale = 40,
       .sigmadelta_gpio = GPIO_SEPIC_IMAX
    };
    sigmadelta_config(&sigmadelta_cfg);
    //------------------------------
    // Setup misc GPIO
    //------------------------------
    gpio_pad_select_gpio( GPIO_LED );
    ESP_ERROR_CHECK( gpio_set_direction( GPIO_LED, GPIO_MODE_OUTPUT ) );
    gpio_pad_select_gpio( GPIO_IBATT_SIGN);
    ESP_ERROR_CHECK( gpio_set_direction( GPIO_IBATT_SIGN, GPIO_MODE_INPUT ) );
    //------------------------------
    // Setup ADC
    //------------------------------
    adc1_config_width( ADC_WIDTH_12Bit );
    adc1_config_channel_atten( ADC_CH_VBATT, ADC_ATTEN_0db );
    adc1_config_channel_atten( ADC_CH_IBATT, ADC_ATTEN_2_5db );       // Full scale: 2.2 V (2.2 A)
}

static void adc_monitor_task(void *pvParameters)
{
    char tempBuff[33];
    int32_t vBattVal, iBattVal, i;
    while (true) {
        vBattVal = 0;
        iBattVal = 0;
        for ( i=0; i<128; i++ ){
            vBattVal += adc1_get_raw( ADC_CH_VBATT );
            iBattVal += adc1_get_raw( ADC_CH_IBATT );
        }
        vBattVal = vBattVal * 1000 / 119600;  // [mV]
        iBattVal = iBattVal * 1000 / 169755;  // [mA]
        if ( gpio_get_level( GPIO_IBATT_SIGN ) ){
            iBattVal *= -1;
        }
        ESP_LOGI( T, "%6d mV,  %6d mA", vBattVal, iBattVal );
        if ( client != 0 ){
            itoa( vBattVal, tempBuff, 10);
            mqtt_publish(client, "veloGen/vbatt", tempBuff, strlen(tempBuff), 0, 0);
            itoa( iBattVal, tempBuff, 10);
            mqtt_publish(client, "veloGen/ibatt", tempBuff, strlen(tempBuff), 0, 0);
        }
        vTaskDelay( 50 );
    }
}

void app_main()
{
    init();
    xTaskCreate(&adc_monitor_task, "adc_monitor_task", 2048, NULL, 5, NULL);
}
