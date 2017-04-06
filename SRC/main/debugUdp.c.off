#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>
#include <stdarg.h>
#include "debugUdp.h"

static const char* TAG = __FILE__;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

int g_Socket = 0xDEAD;
struct sockaddr_in g_servaddr;

static int udpDebugPrintf( const char *format, va_list arg ){
    static char charBuffer[255];
    static  int charLen;
    charLen = vsprintf( charBuffer, format, arg );
    if( charLen <= 0 || g_Socket == 0xDEAD ){
        return 0;
    }
    if ( sendto(g_Socket, charBuffer, charLen, 0, (struct sockaddr *)&g_servaddr, sizeof(g_servaddr)) < 0 ) {
        return 0;
    }
    return charLen;
}

void udp_debug_init(void *pvParameters)
{
    struct hostent *hp;     /* host information */
    // Wait for active IP connection
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");

    /* fill in the server's address and data */
    memset((char*)&g_servaddr, 0, sizeof(g_servaddr));
    g_servaddr.sin_family = AF_INET;
    g_servaddr.sin_port = htons(UDP_DEBUG_PORT);

    /* look up the address of the server given its name */
    hp = gethostbyname(UDP_DEBUG_SERVER);
    if (!hp) {
        ESP_LOGE(TAG, "DNS lookup failed host=%s, error=%d", UDP_DEBUG_SERVER, h_errno);
        return;
    }

    /* put the host's address into the server address structure */
    memcpy((void *)&g_servaddr.sin_addr, hp->h_addr, hp->h_length);
    ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(hp->h_addr) );


    if( (g_Socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
        ESP_LOGE(TAG, "Failed to create UDP socket");
        return;
    }

    esp_log_set_vprintf( udpDebugPrintf );

    vTaskDelete(NULL);
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
     switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits( wifi_event_group, CONNECTED_BIT );
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits( wifi_event_group, CONNECTED_BIT );
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifiInit()
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MODEM) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "Narrenzunft-BE",
            .password = "***REMOVED***",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    xTaskCreate(&udp_debug_init,   "udp_debug_task",   2048, NULL, 5, NULL);
}
