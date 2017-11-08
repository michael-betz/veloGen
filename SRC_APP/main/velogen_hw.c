#include <stdio.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "libesphttpd/esp.h"
#include "libesphttpd/cgiwebsocket.h"
#include "cJSON.h"
#include "driver/ledc.h"
#include "driver/sigmadelta.h"
#include "driver/adc.h"

#include "velogen_hw.h"

static const char *T = "VGEN_HW";

void initVelogen(){
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

void adc_monitor_task(void *pvParameters){
    int32_t vBattVal, iBattVal, i;
    cJSON *jRoot;
    char *jsonString;
    while (true) {
        vBattVal = 0;
        iBattVal = 0;
        for ( i=0; i<128; i++ ){
            vBattVal += adc1_get_raw( ADC_CH_VBATT );
            iBattVal += adc1_get_raw( ADC_CH_IBATT );
        }
        vBattVal = vBattVal * 1000 / 113133 + 391;  // [mV]
        iBattVal = iBattVal * 1000 / 420500 -  35;  // [mA]
        if( iBattVal < 0 ){
            iBattVal = 0;
        }
        if ( gpio_get_level( GPIO_IBATT_SIGN ) ){
            iBattVal *= -1;
        }
        // ESP_LOGI( T, "%6d mV,  %6d mA", vBattVal, iBattVal );
        jRoot = cJSON_CreateObject();
        cJSON_AddNumberToObject( jRoot, "vBattVal", vBattVal );
        cJSON_AddNumberToObject( jRoot, "iBattVal", iBattVal );
        jsonString = cJSON_Print( jRoot );
        cgiWebsockBroadcast("/debug/ws.cgi", jsonString, strlen(jsonString), WEBSOCK_FLAG_NONE);
        free( jsonString );
        cJSON_Delete( jRoot );
        // if ( client != 0 ){
        //     itoa( vBattVal, tempBuff, 10);
        //     mqtt_publish(client, "veloGen/vbatt", tempBuff, strlen(tempBuff), 0, 0);
        //     itoa( iBattVal, tempBuff, 10);
        //     mqtt_publish(client, "veloGen/ibatt", tempBuff, strlen(tempBuff), 0, 0);
        // }
        vTaskDelay( 300/portTICK_PERIOD_MS );
    }
}