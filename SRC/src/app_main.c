// next steps
// * shutdown state machine (look for wifi, try upload)
// * OTA: load firmware from github release

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
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
#include "wifi.h"
#include "ws_logger.h"
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

static const char *T = "MAIN";

int ota_n_written = -1;

/* Event handler for catching system events */
static void
ota_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == ESP_HTTPS_OTA_EVENT) {
        switch (event_id) {
        case ESP_HTTPS_OTA_START:
            ESP_LOGI(T, "OTA started");
            ota_n_written = 0;
            break;
        case ESP_HTTPS_OTA_CONNECTED:
            ESP_LOGI(T, "Connected to server");
            break;
        case ESP_HTTPS_OTA_GET_IMG_DESC:
            ESP_LOGI(T, "Reading Image Description");
            break;
        case ESP_HTTPS_OTA_VERIFY_CHIP_ID:
            ESP_LOGI(T, "Verifying chip id of new image: %d", *(esp_chip_id_t *)event_data);
            break;
        case ESP_HTTPS_OTA_VERIFY_CHIP_REVISION:
            ESP_LOGI(T, "Verifying chip revision of new image: %d", *(esp_chip_id_t *)event_data);
            break;
        case ESP_HTTPS_OTA_DECRYPT_CB:
            ESP_LOGI(T, "Callback to decrypt function");
            break;
        case ESP_HTTPS_OTA_WRITE_FLASH:
            ota_n_written = *(int *)event_data;
            // ESP_LOGD(T, "Writing to flash: %d written", ota_n_written);
            break;
        case ESP_HTTPS_OTA_UPDATE_BOOT_PARTITION:
            ESP_LOGI(T,
                     "Boot partition updated. Next Partition: %d",
                     *(esp_partition_subtype_t *)event_data);
            break;
        case ESP_HTTPS_OTA_FINISH:
            ESP_LOGI(T, "OTA finish");
            ota_n_written = -1;
            break;
        case ESP_HTTPS_OTA_ABORT:
            ESP_LOGI(T, "OTA abort");
            ota_n_written = -1;
            break;
        }
    }
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

    velogen_init();
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true) {
        velogen_loop();

        // Run with a fixed 20 Hz cycle rate
        vTaskDelayUntil(&xLastWakeTime, CYCLE_MS / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static atomic_flag ota_in_progress = ATOMIC_FLAG_INIT;

static void ota_task(void *pvParameters) {
    const char *ota_url = jGetS(getSettings(), "ota_url", NULL);
    if ((wifi_state != WIFI_CONNECTED && wifi_state != WIFI_AP_MODE) || ota_url == NULL)
        goto ota_exit;

    log_w("Pulling OTA update from: %s", ota_url);
    esp_http_client_config_t hconfig = {.url = ota_url,
                                        .skip_cert_common_name_check = true,
                                        .crt_bundle_attach = esp_crt_bundle_attach};
    esp_https_ota_config_t config = {
        .http_config = &hconfig,
        .bulk_flash_erase = true,
        .partial_http_download = true,
        .max_http_request_size = 0,
    };

    E(esp_event_handler_register(ESP_HTTPS_OTA_EVENT, ESP_EVENT_ANY_ID, &ota_event_handler, NULL));

    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        log_i("OTA success. Restarting!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    } else {
        log_e("OTA failed: %d", ret);
    }

ota_exit:
    atomic_flag_clear(&ota_in_progress);
    vTaskDelete(NULL);
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
            cJSON *s = getSettings();
            g_sleepTimeout = jGetI(s, "sleep_timeout", 300) * 1000 / portTICK_PERIOD_MS;
            if (jGetB(s, "reset_wheel_cnt", false))
                g_wheelCnt = 0;
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
        if (!atomic_flag_test_and_set(&ota_in_progress))
            xTaskCreate(ota_task, "ota", 4096, NULL, 0, NULL);
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
