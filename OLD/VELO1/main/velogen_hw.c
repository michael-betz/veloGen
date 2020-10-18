#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "libesphttpd/esp.h"
#include "libesphttpd/cgiwebsocket.h"
#include "cJSON.h"
#include "driver/ledc.h"
#include "driver/sigmadelta.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "driver/pcnt.h"

#include "wifi_startup.h"
#include "velogen_hw.h"

static const char *T = "VGEN_HW";

RTC_DATA_ATTR uint32_t g_wheelCnt;
static int g_dac_value = 0;
static int g_duty_value = 0;
static int g_shutdown_ticks = 0;

// Maximum PWM duty cycle
void set_duty(int duty_value) {
    if(duty_value >= 0 && duty_value <= 127) {
        g_duty_value = duty_value;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty_value);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    }
}

// Peak inductor current
void set_dac(int dac_value) {
    if(dac_value >= -128 && dac_value <= 127){
        g_dac_value = dac_value;
        sigmadelta_set_duty(SIGMADELTA_CHANNEL_0, dac_value);
    }
}

void initVelogen() {
    //------------------------------
    // Load settings
    //------------------------------
    g_dac_value = jGetInt(getSettings(), "dac_value", DAC_VALUE);
    g_duty_value = jGetInt(getSettings(), "duty_value", DUTY_VALUE);
    g_shutdown_ticks = jGetInt(getSettings(), "shut_down_ticks", SHUT_DOWN_TICKS);
    ESP_LOGI(T, "g_dac_value = %d, g_duty_value = %d, g_shutdown_ticks = %d", g_dac_value, g_duty_value, g_shutdown_ticks);

    //------------------------------
    // Setup the PWM hardware (LEDC)
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
        .duty = g_duty_value,
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
       .sigmadelta_duty = g_dac_value,
       .sigmadelta_prescale = 40,
       .sigmadelta_gpio = GPIO_SEPIC_IMAX
    };
    sigmadelta_config(&sigmadelta_cfg);

    //------------------------------
    // Setup misc GPIO
    //------------------------------
    rtc_gpio_deinit( GPIO_SPEED_PLS );
    gpio_pad_select_gpio( GPIO_SPEED_PLS );
    ESP_ERROR_CHECK( gpio_set_direction( GPIO_SPEED_PLS, GPIO_MODE_INPUT ) );
    ESP_ERROR_CHECK( gpio_set_pull_mode( GPIO_SPEED_PLS, GPIO_FLOATING ) );
    gpio_pad_select_gpio( GPIO_LED );
    ESP_ERROR_CHECK( gpio_set_direction( GPIO_LED, GPIO_MODE_OUTPUT ) );
    gpio_pad_select_gpio( GPIO_IBATT_SIGN );
    ESP_ERROR_CHECK( gpio_set_direction( GPIO_IBATT_SIGN, GPIO_MODE_INPUT ) );

    //------------------------------
    // Setup ADC
    //------------------------------
    adc1_config_width( ADC_WIDTH_12Bit );
    adc1_config_channel_atten( ADC_CH_VBATT, ADC_ATTEN_0db );
    adc1_config_channel_atten( ADC_CH_IBATT, ADC_ATTEN_2_5db );       // Full scale: 2.2 V (2.2 A)

    //------------------------------
    // Pulse counter to count wheel rotations
    //------------------------------
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = GPIO_SPEED_PLS,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_UNIT_0,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP, // Keep the primary counter mode if low
        .hctrl_mode = PCNT_MODE_KEEP  // Keep the primary counter mode if high
    };

    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);
    ESP_ERROR_CHECK( gpio_set_pull_mode( GPIO_SPEED_PLS, GPIO_FLOATING ) );
    /* Configure and enable the input filter */
    pcnt_set_filter_value(PCNT_UNIT_0, 500);
    pcnt_filter_enable(PCNT_UNIT_0);
    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    /* Everything is set up, now go to counting */
    pcnt_counter_resume(PCNT_UNIT_0);
}

void adc_monitor_task(void *pvParameters){
    int32_t vBattVal, iBattVal, i, shutDownTimeout=0;
    int16_t pCnt=0, lastCnt=0, diffCnt=0;
    cJSON *jRoot;
    char *jsonString;
    while (true) {
        vBattVal = 0;
        iBattVal = 0;
        for ( i=0; i<128; i++ ){
            vBattVal += adc1_get_raw(ADC_CH_VBATT);
            iBattVal += adc1_get_raw(ADC_CH_IBATT);
        }
        vBattVal = vBattVal * 1000 / 113133 + 295;  // [mV]
        iBattVal = iBattVal * 1000 / 420500 -  35;  // [mA]
        if(iBattVal < 0){
            iBattVal = 0;
        }
        if (gpio_get_level(GPIO_IBATT_SIGN)) {
            iBattVal *= -1;
        }
        if(pcnt_get_counter_value(PCNT_UNIT_0, &pCnt) == ESP_OK) {
            diffCnt = pCnt - lastCnt;
            if (diffCnt == 0) {
                if(++shutDownTimeout > g_shutdown_ticks) {
                    ESP_LOGW(T, "Going to deep sleep ...");
                    shutDownTimeout = 0;
                    // go to deep sleep
                    rtc_gpio_pullup_dis(GPIO_SPEED_PLS);
                    rtc_gpio_pulldown_dis(GPIO_SPEED_PLS);
                    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(GPIO_SPEED_PLS, 1));
                    esp_deep_sleep_start();
                }
            } else {
                shutDownTimeout = 0;
                g_wheelCnt += diffCnt;
            }
            lastCnt = pCnt;
        }
        gpio_set_level(GPIO_LED, gpio_get_level(GPIO_SPEED_PLS));
        // ESP_LOGI( T, "%6d mV,  %6d mA,  %6d pls", vBattVal, iBattVal, pCnt );
        jRoot = cJSON_CreateObject();
        cJSON_AddNumberToObject(jRoot, "time", time(NULL));
        cJSON_AddNumberToObject(jRoot, "tick_count", xTaskGetTickCount());
        cJSON_AddNumberToObject(jRoot, "dac_value", g_dac_value);
        cJSON_AddNumberToObject(jRoot, "duty_value", g_duty_value);
        cJSON_AddNumberToObject(jRoot, "vBattVal", vBattVal);
        cJSON_AddNumberToObject(jRoot, "iBattVal", iBattVal);
        cJSON_AddNumberToObject(jRoot, "wheelCnt", g_wheelCnt);
        jsonString = cJSON_Print(jRoot);
        cgiWebsockBroadcast("/ws.cgi", jsonString, strlen(jsonString), WEBSOCK_FLAG_NONE);
        free(jsonString);
        cJSON_Delete(jRoot);
        // if ( client != 0 ){
        //     itoa( vBattVal, tempBuff, 10);
        //     mqtt_publish(client, "veloGen/vbatt", tempBuff, strlen(tempBuff), 0, 0);
        //     itoa( iBattVal, tempBuff, 10);
        //     mqtt_publish(client, "veloGen/ibatt", tempBuff, strlen(tempBuff), 0, 0);
        // }
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}
