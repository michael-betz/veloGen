/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"

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
#include "tftp.h"
#include "cgi.h"
#include "cgi-test.h"

static const char *T = "VELOEXP";

static int wsDebugPrintf( const char *format, va_list arg ){
    static char charBuffer[512];
    static  int charLen;
    charLen = vsprintf( charBuffer, format, arg );
    if( charLen <= 0 ){
        return 0;
    }
    charBuffer[511] = '\0';
    cgiWebsockBroadcast("/debug/ws.cgi", charBuffer, charLen, WEBSOCK_FLAG_NONE);
    // Output to UART as well
    printf( "%s", charBuffer );
    return charLen;
}

//Debugging Websocket connected.
static void debugWsConnect(Websock *ws) {
    esp_log_set_vprintf( wsDebugPrintf );
    ESP_LOGI(T,"Client connected to debug websocket");
}

const HttpdBuiltInUrl builtInUrls[]={
    {"*", cgiRedirectApClientToHostname, HOSTNAME},
    {"/", cgiRedirect, "/index.tpl"},
    {"/led.tpl", cgiEspFsTemplate, tplLed},
    {"/index.tpl", cgiEspFsTemplate, tplCounter},
    {"/led.cgi", cgiLed, NULL},

    {"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
    {"/wifi/", cgiRedirect, "/wifi/wifi.tpl"},
    {"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
    {"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
    {"/wifi/connect.cgi", cgiWiFiConnect, NULL},
    {"/wifi/connstatus.cgi", cgiWiFiConnStatus, NULL},
    {"/wifi/setmode.cgi", cgiWiFiSetMode, NULL},

    {"/reboot", cgiRebootFirmware, NULL},

    {"/test", cgiRedirect, "/test/index.html"},
    {"/test/", cgiRedirect, "/test/index.html"},
    {"/test/test.cgi", cgiTestbed, NULL},

    {"/debug",          cgiRedirect, "/debug/index.html"},
    {"/debug/",         cgiRedirect, "/debug/index.html"},
    {"/debug/ws.cgi",   cgiWebsocket, debugWsConnect },

    {"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
    {NULL, NULL, NULL}
};

void clearBootFlag(){
    // next reboot will go back in previous firmware
    const esp_partition_t* otaselpart=esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, NULL);
    if( otaselpart ){
        // Kill the flags in otadata to boot from part0
        spi_flash_erase_sector(otaselpart->address/0x1000);
        spi_flash_erase_sector(otaselpart->address/0x1000+1);
        printf("otadata erased\n");
    } else {
        printf("!!! otadata not found !!!\n");
    }
}

void app_main()
{
    clearBootFlag();

    int temp;
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
    // Start tftp server
    //------------------------------
    // xTaskCreate(&tFtpServerTask, "tftp_server_task", 4096, NULL, 1, NULL);

    //------------------------------
    // Start http server
    //------------------------------
    ESP_ERROR_CHECK( espFsInit((void*)(webpages_espfs_start)) );
    ESP_ERROR_CHECK( httpdInit(builtInUrls, 80, 0) );
    ESP_LOGW(T,"RAM left %d\n", esp_get_free_heap_size() );
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
