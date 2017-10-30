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

#define EXPERIMENTAL 1

#if EXPERIMENTAL
    static const char *T = "VELOEXP";
#else
    static const char *T = "VELORECOVER";
#endif

//Broadcast the uptime in seconds every second over connected websockets
// static void websocketBcast(void *arg) {
//     static int ctr=0;
//     char buff[128];
//     while(1) {
//         ctr++;
//         sprintf(buff, "Up for %d minutes %d seconds!\n", ctr/60, ctr%60);
//         cgiWebsockBroadcast("/websocket/ws.cgi", buff, strlen(buff), WEBSOCK_FLAG_NONE);
//         vTaskDelay(1000/portTICK_RATE_MS);
//     }
// }

//On reception of a message, send "You sent: " plus whatever the other side sent
static void myWebsocketRecv(Websock *ws, char *data, int len, int flags) {
    int i;
    char buff[128];
    sprintf(buff, "You sent: ");
    for (i=0; i<len; i++) buff[i+10]=data[i];
    buff[i+10]=0;
    cgiWebsocketSend(ws, buff, strlen(buff), WEBSOCK_FLAG_NONE);
}

//Websocket connected. Install reception handler and send welcome message.
static void myWebsocketConnect(Websock *ws) {
    ws->recvCb=myWebsocketRecv;
    cgiWebsocketSend(ws, "Hi, Websocket!", 14, WEBSOCK_FLAG_NONE);
}

CgiUploadFlashDef uploadParams={
    .type=CGIFLASH_TYPE_FW,
    .fw1Pos=0x110000,
    .fw2Pos=0x110000,
    .fwSize=0x100000,
    .tagName="main1"
};

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

    {"/flash/", cgiRedirect, "/flash/index.html"},
    // {"/flash/next", cgiGetFirmwareNext, &uploadParams},
    {"/flash/upload", cgiUploadFirmware, &uploadParams},
    {"/flash/reboot", cgiRebootFirmware, NULL},


    {"/websocket/ws.cgi", cgiWebsocket, myWebsocketConnect},

    {"/test", cgiRedirect, "/test/index.html"},
    {"/test/", cgiRedirect, "/test/index.html"},
    {"/test/test.cgi", cgiTestbed, NULL},

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
    #ifdef EXPERIMENTAL
        clearBootFlag();
    #endif
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
    // xTaskCreate( websocketBcast, "wsbcast", 4096, NULL, 1, NULL);
    ESP_LOGW(T,"RAM left %d\n", esp_get_free_heap_size() );
}

// Print a pretty hex-dump on the debug out
static void hexDump( uint8_t *buffer, uint16_t nBytes ){
    for( uint16_t i=0; i<nBytes; i++ ){
        if( (nBytes>16) && ((i%16)==0) ){
            printf("\n    %04x: ", i);
        }
        printf("%02x ", *buffer++);
    }
    printf("\n");
}
