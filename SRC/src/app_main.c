// next steps
// * shutdown state machine (look for wifi, try upload)
// * OTA: load firmware from github release

#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <time.h>

#include "json_settings.h"
#include "velo.h"

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
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_CACHE", ESP_LOG_DEBUG);

    // report status
    ESP_LOGI(T,
             "reset reason: %u, heap: %lu, min_heap: %lu",
             esp_reset_reason(),
             esp_get_free_heap_size(),
             esp_get_minimum_free_heap_size());

    // Mount FS for *.html and defaults.json
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/lfs",
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
    set_settings_file("/lfs/settings.json", "/lfs/default_settings.json");

    // xTaskCreatePinnedToCore(velo_task, "velo_task", 4096, NULL, 1, NULL, 1);
    velo_task(NULL);
}
