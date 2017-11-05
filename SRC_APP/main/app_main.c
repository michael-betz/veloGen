/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <errno.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

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
#include "wifi_startup.h"
#include "cgi.h"
#include "web_console.h"

static const char *T = "VELOEXP";

CgiUploadFlashDef uploadParams={
    .type=CGIFLASH_TYPE_FW,
    .fw1Pos=0x010000,
    .fw2Pos=0x0D0000,
    .fwSize=0x0C0000,
    .tagName="main1"
};

//Debugging Websocket connected.
static void debugWsConnect(Websock *ws) {
    ESP_LOGI(T,"Client connected to debug websocket");
}

const HttpdBuiltInUrl builtInUrls[]={
    {"/", cgiRedirect, "/index.html"},

    {"/debug",         cgiRedirect, "/debug/index.html"},
    {"/debug/log",     cgiWebsocket, debugWsConnect },
    {"/debug/ws.cgi",  cgiWebsocket, debugWsConnect },

    {"/flash",         cgiRedirect,       "/flash/index.html" },
    {"/flash/upload",  cgiUploadFirmware, &uploadParams },
    {"/reboot",        cgiRebootFirmware, NULL },

    {"/S/*",           cgiEspSPIFFSHook, NULL}, //Catch-all cgi function for the SPIFFS filesystem
    {"*",              cgiEspFsHook,     NULL}, //Catch-all cgi function for the static filesystem
    {NULL, NULL, NULL}
};

void app_main()
{
    initFs();
    ESP_LOGW(T,"RAM left %d\n", esp_get_free_heap_size());

    //------------------------------
    // Setup Wifi
    //------------------------------
    ESP_ERROR_CHECK( nvs_flash_init() );
    wifi_conn_init();

    //------------------------------
    // Setup misc GPIO
    //------------------------------
    gpio_pad_select_gpio( GPIO_LED );
    ESP_ERROR_CHECK( gpio_set_direction( GPIO_LED, GPIO_MODE_OUTPUT ) );

    //------------------------------
    // Start http server
    //------------------------------
    ESP_ERROR_CHECK( espFsInit((void*)(webpages_espfs_start)) );
    ESP_ERROR_CHECK( httpdInit(builtInUrls, 80, 0) );
    esp_log_set_vprintf( wsDebugPrintf );
    ESP_LOGW(T,"FW 1, RAM left %d\n", esp_get_free_heap_size() );
}

// // Print a pretty hex-dump on the debug out
// static void hexDump( uint8_t *buffer, uint16_t nBytes ){
//     for( uint16_t i=0; i<nBytes; i++ ){
//         if( (nBytes>16) && ((i%16)==0) ){
//             printf("\n    %04x: ", i);
//         }
//         printf("%02x ", *buffer++);
//     }
//     printf("\n");
// }
