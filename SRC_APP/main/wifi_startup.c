
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <string.h>
#include <errno.h>
#include "apps/sntp/sntp.h"
#include "cJSON.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_spiffs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "wifi_startup.h"

static const char *T = "WIFI_STARTUP";
EventGroupHandle_t wifi_event_group;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{  
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK( tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, HOSTNAME) );
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            if (!sntp_enabled()){
                ESP_LOGI(T, "Initializing SNTP");
                sntp_setoperatingmode(SNTP_OPMODE_POLL);
                sntp_setservername(0, "pool.ntp.org");
                sntp_init();
            }
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
               auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

char *readFileDyn( char* fName, int *fSize ){
    FILE *f = fopen(fName, "rb");
    if ( !f ) {
        ESP_LOGE( T, " fopen( %s ) failed: %s", fName, strerror(errno) );
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);
    // ESP_LOGI(T, "filesize: %d", fsize );
    char *string = malloc( fsize + 1 );
    if ( !string ) {
        ESP_LOGE( T, "malloc( %d ) failed: %s", fsize+1, strerror(errno) );
        return NULL;
    }
    fread(string, fsize, 1, f);
    fclose(f);
    string[fsize] = 0;
    if( fSize )
        *fSize = fsize;
    return string;
}

cJSON *readJsonDyn( char* fName ){
    cJSON *root;
    char *txtData = readFileDyn( fName, NULL );
    if( !txtData ){
        return NULL;
    }
    root = cJSON_Parse( txtData );
    free( txtData );
    return root;
}

#ifndef cJSON_ArrayForEach
    /* Macro for iterating over an array or object */
    #define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
#endif

void wifiConnectionTask(void *pvParameters){
    int i;
    uint16_t foundNaps=0;
    wifi_ap_record_t *foundAps=NULL, *currAp=NULL;
    cJSON *jRoot=NULL, *jWifi=NULL, *jTemp=NULL;
    wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config ) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    while(1){
        //---------------------------------------------------
        // Scan for wifis around
        //---------------------------------------------------
        wifi_scan_config_t scanConfig = { //Define a scan filter
            .ssid = 0,
            .bssid = 0,
            .channel = 0,
            .show_hidden = false,
            .scan_type = WIFI_SCAN_TYPE_ACTIVE
        };
        ESP_ERROR_CHECK( esp_wifi_scan_start( &scanConfig, 1 ) );
        // BLocks for 1500 ms
        ESP_ERROR_CHECK( esp_wifi_scan_get_ap_num( &foundNaps ) );
        // copy AP data in local memory
        foundAps = (wifi_ap_record_t*) malloc( sizeof(wifi_ap_record_t)*foundNaps );
        assert( foundNaps );
        ESP_ERROR_CHECK( esp_wifi_scan_get_ap_records( &foundNaps, foundAps ) );
        ESP_LOGD(T, "\n--------------------- %d wifis -----------------------", foundNaps );        
        for( i=0,currAp=foundAps; i<foundNaps; i++ ){
            ESP_LOGD(T, "ch: %2d, ssid: %16s, bssid: %02x:%02x:%02x:%02x:%02x:%02x, rssi: %d", currAp->primary, (char*)currAp->ssid, currAp->bssid[0], currAp->bssid[1], currAp->bssid[2], currAp->bssid[3], currAp->bssid[4], currAp->bssid[5], currAp->rssi );
            currAp++;
        }
        //---------------------------------------------------
        // See if there are any known wifis
        //---------------------------------------------------
        // iterate trhough all keys of json file
        // char *current_key=NULL;
        // if( (jRoot = readJsonDyn("/S/knownWifis.json")) ){
        //     cJSON_ArrayForEach(jWifi, jRoot) {
        //         if ( (current_key=jWifi->string) ){
        //             for( i=0,currAp=foundAps; i<foundNaps; i++ ){
        //                 if ( strcmp( current_key, (char*)currAp->ssid ) == 0 ){
        //                     ESP_LOGI(T,"match: %s, %s", current_key, currAp->ssid);
        //                 }
        //                 currAp++;
        //             }
        //         }
        //     }
        //     free( jRoot );
        // }
        // Index json file by key (ssid)
        if( (jRoot = readJsonDyn("/S/knownWifis.json")) ){
            for( i=0,currAp=foundAps; i<foundNaps; i++ ){
                if ( (jWifi = cJSON_GetObjectItem( jRoot, (char*)currAp->ssid)) ){
                    break;
                }
                currAp++;
            }
            if( jWifi ){
                if( !(jTemp = cJSON_GetObjectItem( jWifi, "pw" )) || (jTemp->type!=cJSON_String) ){
                    ESP_LOGE(T, "Wifi pw undefined");
                }
                ESP_LOGW(T,"match: %s, trying to connect ...", jWifi->string );
                strcpy( (char*)wifi_config.sta.ssid,     jWifi->string );
                strcpy( (char*)wifi_config.sta.password, jTemp->valuestring );
                ESP_ERROR_CHECK( esp_wifi_stop() );
                ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config ) );
                ESP_ERROR_CHECK( esp_wifi_start() );
                cJSON_Delete( jRoot );
                free( foundAps );
                break;
            } else {
                ESP_LOGW(T,"No known wifi found");
            }
            cJSON_Delete( jRoot );
        }
        free( foundAps );
    }
    vTaskDelete( NULL );
}

void wifi_conn_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    xTaskCreate(&wifiConnectionTask, "wifiConnectionTask", 2048, NULL, 5, NULL);
}

void wifi_disable(){
    esp_wifi_stop();
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
}