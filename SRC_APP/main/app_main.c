/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"

#include "rom/cache.h"
#include "rom/ets_sys.h"
#include "rom/spi_flash.h"
#include "rom/crc.h"
#include "rom/rtc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "libesphttpd/esp.h"
#include "libesphttpd/espfs.h"
#include "libesphttpd/httpdespfs.h"
#include "libesphttpd/webpages-espfs.h"
#include "libesphttpd/cgiflash.h"
#include "libesphttpd/cgiwifi.h"
#include "libesphttpd/cgiwebsocket.h"

#include "app_main.h"
#include "velogen_hw.h"
#include "wifi_startup.h"
#include "cgi.h"
#include "web_console.h"
#include "velogen_hw.h"

static const char *T = "MAIN_APP";

CgiUploadFlashDef uploadParams={
    .type=CGIFLASH_TYPE_FW,
    .fw1Pos=0x010000,
    .fw2Pos=0x0D0000,
    .fwSize=0x0C0000,
    .tagName="main1"
};

//Debugging Websocket connected.
static void debugWsConnect(Websock *ws) {
    uint8_t *rip = ws->conn->remote_ip;
    ESP_LOGI(T,"WS-client: %d.%d.%d.%d", rip[0], rip[1], rip[2], rip[3] );
}

//------------------------------
// Setup webserver
//------------------------------
const HttpdBuiltInUrl builtInUrls[]={
    {"/", cgiRedirect, "/index.html"},

    {"/debug",         cgiRedirect,             "/debug/index.html" },
    {"/debug/ws.cgi",  cgiWebsocket,            debugWsConnect },

    {"/flash",         cgiRedirect,             "/flash/index.html" },
    {"/flash/upload",  cgiUploadFirmware,       &uploadParams },
    {"/reboot",        cgiRebootFirmware,       NULL },

    {"/log.txt",       cgiEspRTC_LOG,           NULL },
    {"/S",             cgiEspSPIFFSListHook,    NULL },
    {"/S/*",           cgiEspSPIFFSHook,        NULL }, //Catch-all cgi function for the SPIFFS filesystem
    {"*",              cgiEspFsHook,            NULL }, //Catch-all cgi function for the static filesystem
    {NULL, NULL, NULL}
};

void app_main()
{
    //------------------------------
    // Enable log file
    //------------------------------
    initFs();
    memset( rtcLogBuffer, '\r', LOG_FILE_SIZE );
    esp_log_set_vprintf( wsDebugPrintf );
    
    initVelogen();
    gpio_set_level( GPIO_LED, 1 );

    //------------------------------
    // Print chip information
    //------------------------------
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(T,"This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores, (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(T,"silicon revision %d, ", chip_info.revision);
    ESP_LOGI(T,"%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024), (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    ESP_LOGI(T,"RAM left %d\n", esp_get_free_heap_size());

    //------------------------------
    // Setup Wifi
    //------------------------------
    ESP_ERROR_CHECK( nvs_flash_init() );
    wifi_conn_init();

    //------------------------------
    // Start http server
    //------------------------------
    ESP_ERROR_CHECK( espFsInit((void*)(webpages_espfs_start)) );
    ESP_ERROR_CHECK( httpdInit(builtInUrls, 80, 0) );
    ESP_LOGW(T,"FW 1, RAM left %d\n", esp_get_free_heap_size() );

    //------------------------------
    // Set the clock / print time
    //------------------------------
    // Set timezone to Eastern Standard Time and print local time
    // setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    // tzset();
    time_t now = 0;
    struct tm timeinfo = { 0 };
    char strftime_buf[64];
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    time(&now);       
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(T, "UTC: %s", strftime_buf);

    // while(1){
    //     // esp_sleep_enable_timer_wakeup(5 * 1000 * 1000);
    //     vTaskDelay(20 * 1000 / portTICK_PERIOD_MS);
    //     ESP_LOGI(T,"Disabling WIFI");
    //     ESP_ERROR_CHECK(esp_wifi_stop());
    //     gpio_set_level( GPIO_LED, 0 );
    //     vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
    //     ESP_LOGI(T,"Enabling Wifi");
    //     ESP_ERROR_CHECK(esp_wifi_start());
    //     gpio_set_level( GPIO_LED, 1 );
    // }

    ESP_LOGI(T,"Starting ADC monitoring task");
    xTaskCreate(&adc_monitor_task, "adc_monitor_task", 2048, NULL, 5, NULL);
    
    // wifi_disable();
    // esp_deep_sleep_start();
}