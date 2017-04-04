#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#define UDP_DEBUG_SERVER "kimchi"
#define UDP_DEBUG_PORT 	  4711

static const char* TAG = "main.c";

#define ADC_VBATT_CHANNEL    3
#define ADC_VBATT_R1       110
#define ADC_VBATT_R2        36
#define ADC_GET_VBATT(cnt) ( ((uint32_t)cnt) * 1100 * (ADC_VBATT_R1+ADC_VBATT_R2) / ADC_VBATT_R2 / 4095 ) //[mV]
// For some reason (maybe my chinese resistors? this is more accurate
//#define ADC_GET_VBATT(cnt) ( ((uint32_t)cnt) * 1048 * (ADC_VBATT_R1+ADC_VBATT_R2) / ADC_VBATT_R2 / 4095 ) //[mV]

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR uint32_t boot_count = 0;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
#define CONNECTED_BIT BIT0

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

int g_Socket = 0xDEAD;
struct sockaddr_in g_servaddr;

static int udpDebugPrintf( const char *format, va_list arg ){
	static char charBuffer[255];
	static 	int charLen;
	charLen = vsprintf( charBuffer, format, arg );
	if( charLen <= 0 || g_Socket == 0xDEAD ){
		return 0;
	}
	if ( sendto(g_Socket, charBuffer, charLen, 0, (struct sockaddr *)&g_servaddr, sizeof(g_servaddr)) < 0 ) {
		return 0;
	}
	return charLen;
}

// Display UDP log with netcat -ul 4711
static void udp_debug_init(void *pvParameters)
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


static void adc_monitor_task(void *pvParameters)
{
    adc1_config_width( ADC_WIDTH_12Bit );
    adc1_config_channel_atten( ADC_VBATT_CHANNEL, ADC_ATTEN_0db );

    uint32_t adcVal, i;
    while (true) {
        adcVal = 0;
        for ( i=0; i<32; i++ ){
        	adcVal += adc1_get_voltage(ADC_VBATT_CHANNEL);
        }
        adcVal /= 32;
        ESP_LOGI( TAG, "%4d  v_batt = %d mV (%d)", boot_count++, ADC_GET_VBATT(adcVal), adcVal );

        vTaskDelay( 2500 );
//        ESP_ERROR_CHECK( esp_deep_sleep_enable_timer_wakeup( 10 * 1000000 ) );
//        esp_deep_sleep_start();
    }
}

// Seems like we get 9 pulses per revolution
#define GEN_GPIO GPIO_NUM_5
#define LED_GPIO GPIO_NUM_12

static void gen_monitor_task(void *pvParameters)
{
	uint8_t val=0, temp=0;
	ESP_ERROR_CHECK( gpio_set_direction(GEN_GPIO, GPIO_MODE_INPUT) );
	ESP_ERROR_CHECK( gpio_set_pull_mode(GEN_GPIO, GPIO_FLOATING)   );

    while (true) {
    	temp = gpio_get_level(GEN_GPIO);
    	if(  temp != val ){
    		val = temp;
    		gpio_set_level( GPIO_NUM_12, temp );
	        ESP_LOGI( TAG, "%d", temp );
    	}
        vTaskDelay( 2 );
    }
}

void app_main(void)
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	// Init LED port
	gpio_pad_select_gpio( GPIO_NUM_12 );
	ESP_ERROR_CHECK( gpio_set_direction( GPIO_NUM_12, GPIO_MODE_OUTPUT ) );
	// INit ticker port
	ESP_ERROR_CHECK( gpio_set_direction(GEN_GPIO, GPIO_MODE_INPUT) );
	ESP_ERROR_CHECK( gpio_set_pull_mode(GEN_GPIO, GPIO_FLOATING)   );
//	while(1){
//		uint8_t temp = gpio_get_level( GEN_GPIO );
//		gpio_set_level( LED_GPIO, temp );
//		vTaskDelay( 1 );
//	}

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
    xTaskCreate(&adc_monitor_task, "adc_monitor_task", 2048, NULL, 5, NULL);
    xTaskCreate(&gen_monitor_task, "gen_monitor_task", 2048, NULL, 5, NULL);
}

