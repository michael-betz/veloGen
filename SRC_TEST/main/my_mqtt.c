#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "driver/ledc.h"
#include "driver/sigmadelta.h"
#include "esp_log.h"
#include "mqtt.h"

#include "app_main.h"
#include "my_mqtt.h"

static const char *T = "VELOMQTT";
mqtt_client *client = 0;
EventGroupHandle_t wifi_event_group;

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

void disconnected_cb(void *self, void *params)
{
    client = 0;
    ESP_LOGI(T, "MQTT_disconnected_cb");
}

void reconnect_cb(void *self, void *params)
{
    ESP_LOGI(T, "MQTT_reconnect_cb");
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
    .auto_reconnect = true,
    .lwt_topic = "veloGen/status",
    .lwt_msg = "offline",
    .lwt_msg_len = 7,
    .lwt_qos = 0,
    .lwt_retain = 1,
    .connected_cb = connected_cb,
    .disconnected_cb = disconnected_cb,
    // .reconnect_cb = reconnect_cb,
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

void wifi_conn_init(void)
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