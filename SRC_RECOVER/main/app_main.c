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
// #include "tftp.h"

static const char *T = "VELORECOVER";

CgiUploadFlashDef uploadParams={
    .type=CGIFLASH_TYPE_FW,
    .fw1Pos=0x080000,
    .fw2Pos=0x080000,
    .fwSize=0x100000,
    .tagName="main1"
};

const HttpdBuiltInUrl builtInUrls[]={
    {"*",               cgiRedirectApClientToHostname,  HOSTNAME },
    {"/",               cgiRedirect,       "/flash/index.html" },
    {"/flash/",         cgiRedirect,       "/flash/index.html" },
    {"/flash/upload",   cgiUploadFirmware, &uploadParams },
    {"/flash/reboot",   cgiRebootFirmware, NULL },
    {"*",               cgiEspFsHook,      NULL }, //Catch-all cgi function for the filesystem
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
    // clearBootFlag();
    //------------------------------
    // Setup Wifi
    //------------------------------
    ESP_ERROR_CHECK( nvs_flash_init() );
    wifi_conn_init();
    
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