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

#include "driver/ledc.h"
#include "driver/sigmadelta.h"
#include "driver/adc.h"

#include "app_main.h"

const char *T = "VELOGEN";

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

mqtt_client *client = 0;

void connected_cb(void *self, void *params)
{
    char tempBuff[32];
    client = (mqtt_client *)self;
    mqtt_publish(client, "veloGen/status", "online", 6, 0, 1);
    itoa( STARTUP_DAC_VALUE, tempBuff, 10);
    mqtt_publish(client, "veloGen/dacValue", tempBuff, strlen(tempBuff), 0, 0);
    itoa( STARTUP_DUTY_VALUE, tempBuff, 10);
    mqtt_publish(client, "veloGen/dutyValue", tempBuff, strlen(tempBuff), 0, 0);
    mqtt_subscribe(client, "veloGen/#",  0);
}

int isTopicMatch( mqtt_event_data_t *event_data, const char *topic )
{
    return( strncmp( event_data->topic, topic, event_data->topic_length ) == 0 );
}

long extractLong( mqtt_event_data_t *event_data )
{
    char tempBuff[32];
    memset( tempBuff, 0, 32 );
    if (event_data->data_length >= 32){
        return 0;
    }
    memcpy( tempBuff, event_data->data, event_data->data_length );
    return( strtol( tempBuff, NULL, 0 ) );
}

void data_cb(void *self, void *params)
{
    long tempValue;
    mqtt_client *client = (mqtt_client *)self;
    mqtt_event_data_t *ed = (mqtt_event_data_t *)params;
    if(ed->data_offset==0) {
        if( isTopicMatch(ed,"veloGen/dacValue") ){
            tempValue = extractLong(ed);
            ESP_LOGI( T, "DAC %ld", tempValue );
            sigmadelta_set_duty(SIGMADELTA_CHANNEL_0, tempValue);
        }
        if( isTopicMatch(ed,"veloGen/dutyCycle") ){
            tempValue = extractLong(ed);
            ESP_LOGI( T, "DUTY %ld", tempValue );
            ledc_set_duty( LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, tempValue );
            ledc_update_duty( LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 );
        }     
        if( isTopicMatch(ed,"veloGen/led") ){
            gpio_set_level( GPIO_LED, strncmp(ed->data,"true",4)==0 );
        }
    }
}

mqtt_settings settings = {
    .host = "roesti",
#if defined(CONFIG_MQTT_SECURITY_ON)
    .port = 8883, // encrypted
#else
    .port = 1883, // unencrypted
#endif
    .client_id = "veloGenClient",
    .clean_session = 1,
    .keepalive = 30,
    .lwt_topic = "veloGen/status",
    .lwt_msg = "offline",
    .lwt_msg_len = 7,
    .lwt_qos = 0,
    .lwt_retain = 1,
    .connected_cb = connected_cb,
    .data_cb = data_cb
};

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            mqtt_start(&settings);
            //init app here
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            esp_wifi_connect();
            mqtt_stop();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_conn_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    // ESP_LOGI(T, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
    ESP_ERROR_CHECK(esp_wifi_start());
}

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
        vTaskDelay( 500 );
    }
}

void app_main()
{
    init();
    xTaskCreate(&adc_monitor_task, "adc_monitor_task", 2048, NULL, 5, NULL);
}
