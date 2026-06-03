// next steps
// * shutdown state machine (look for wifi, try upload)
// * OTA: load firmware from github release

#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "json_settings.h"
#include "main.h"
#include "mqtt_cache.h"
#include "velo.h"
#include "ws_logger.h"
#include <stdio.h>
#include <time.h>

static const char *T = "MAIN";

static void velo_task(void *args) {
    velogen_init();
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true) {
        velogen_loop();

        // Run with a fixed 20 Hz cycle rate
        vTaskDelayUntil(&xLastWakeTime, CYCLE_MS / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main() {
    init_ws_logger();

    // report status
    ESP_LOGI(T,
             "reset reason: %u, heap: %lu, min_heap: %lu",
             esp_reset_reason(),
             esp_get_free_heap_size(),
             esp_get_minimum_free_heap_size());

    // Mount FS for *.html and defaults.json
    esp_vfs_littlefs_conf_t conf = {
        .base_path = F_PREFIX,
        .partition_label = "filesys",
        .format_if_mount_failed = false,
        .dont_mount = false,
    };

    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(T, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(T, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else {
        ESP_LOGI(T, "Partition size: total: %d, used: %d", total, used);
    }

    // Load settings.json from SPIFFS, try to create file if it doesn't exist
    set_settings_file(F_PREFIX "/settings.json", F_PREFIX "/default_settings.json");
    init_log_levels();

    velo_task(NULL);
}

esp_err_t ws_callback(httpd_req_t *req, httpd_ws_frame_t *wsf) {
    if (wsf == NULL || req == NULL)
        return ESP_ERR_INVALID_STATE;

    size_t len = wsf->len;
    uint8_t *payload = wsf->payload;

    static char ret_buffer[64];
    int ret_len = -1;

    if (len < 1 || wsf->type != HTTPD_WS_TYPE_TEXT)
        return ESP_ERR_INVALID_ARG;

    ESP_LOGV(T, "ws_callback(%c, %d)", payload[0], len);

    switch (payload[0]) {
    // read RTC log buffer
    case 'a':
        if (len > 1 && payload[1] == '1')
            // dump complete log buffer
            wsDumpRtc(req, true);
        else
            // dump updates only
            wsDumpRtc(req, false);
        break;

    // read / write settings.json
    case 'b':
        settings_ws_handler(req, &payload[1], len - 1);
        if (len > 1) {
            // Some parameters can be updated without a reboot
            init_log_levels();
            cache_init();
        }
        break;

    // reboot ESP32
    case 'r':
        esp_wifi_disconnect();
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_restart();
        break;

    // trigger esp_https_ota()
    case 'u':
        run_ota_update = true;
        break;

    // Report status and heap usage
    case 'h':
        wifi_ap_record_t wifidata;
        int rssi = 0;
        if (esp_wifi_sta_get_ap_info(&wifidata) == ESP_OK)
            rssi = wifidata.rssi;

        ret_len = snprintf(ret_buffer,
                           sizeof(ret_buffer),
                           "h,%zu,%zu,%zu,%zu,%zu,%zu,%u,%d,%u,%u",
                           // heap stuff
                           heap_caps_get_free_size(MALLOC_CAP_8BIT),
                           heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
                           heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
                           heap_caps_get_free_size(MALLOC_CAP_32BIT),
                           heap_caps_get_largest_free_block(MALLOC_CAP_32BIT),
                           heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT),
                           // uptime in [s]
                           (unsigned)(esp_timer_get_time() / 1000 / 1000),
                           // wifi strength [dBm]
                           rssi,
                           // Average CPU load of the DAC streaming CPU [%]
                           0,
                           // Number of I2C buffer underflows since boot
                           0);
        break;
    }

    // Send reply if needed
    if (ret_len >= 0) {
        if (ret_len > sizeof(ret_buffer)) {
            ESP_LOGE(T, "ret_buffer overflowed! Response dropped");
            return ESP_ERR_NO_MEM;
        }

        httpd_ws_frame_t ret_wsf = {0};
        ret_wsf.type = HTTPD_WS_TYPE_TEXT;
        ret_wsf.payload = (uint8_t *)ret_buffer;
        ret_wsf.len = ret_len;
        httpd_ws_send_frame(req, &ret_wsf);
    }

    return ESP_OK;
}
