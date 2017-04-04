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

static const char* TAG = "main.c";

#define UDP_DEBUG_SERVER "kimchi"
#define UDP_DEBUG_PORT 	  4711

// Seems like we get 9 pulses per revolution
#define GEN_GPIO GPIO_NUM_5
#define LED_GPIO GPIO_NUM_12

// ADC (VBatt) calibration constants
#define ADC_VBATT_CHANNEL    3
#define ADC_M            97245
#define ADC_B         28531865
#define ADC_D           100000
#define ADC_GET_VBATT(cnt) ( ( (uint32_t)cnt * ADC_M + ADC_B ) / ADC_D ) //[mV]

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR uint32_t boot_count = 0;

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

static void gen_monitor_task(void *pvParameters)
{
	uint8_t val=0, temp=0;
	ESP_ERROR_CHECK( gpio_set_direction(GEN_GPIO, GPIO_MODE_INPUT) );
	ESP_ERROR_CHECK( gpio_set_pull_mode(GEN_GPIO, GPIO_FLOATING)   );

    while (true) {
    	temp = gpio_get_level(GEN_GPIO);
    	if(  temp != val ){
    		val = temp;
    		gpio_set_level( LED_GPIO, temp );
	        ESP_LOGI( TAG, "%d", temp );
    	}
        vTaskDelay( 2 );
    }
}

void app_main(void)
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	// Init LED port
	gpio_pad_select_gpio( LED_GPIO );
	ESP_ERROR_CHECK( gpio_set_direction( LED_GPIO, GPIO_MODE_OUTPUT ) );

//	while(1){
//		uint8_t temp = gpio_get_level( GEN_GPIO );
//		gpio_set_level( LED_GPIO, temp );
//		vTaskDelay( 1 );
//	}

    xTaskCreate(&udp_debug_init,   "udp_debug_task",   2048, NULL, 5, NULL);
    xTaskCreate(&adc_monitor_task, "adc_monitor_task", 2048, NULL, 5, NULL);
    xTaskCreate(&gen_monitor_task, "gen_monitor_task", 2048, NULL, 5, NULL);
}

