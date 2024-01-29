// next steps
// * shutdown state machine (look for wifi, try upload)
// * per trip screens
//   * energy / charge / distance / avg speed
// 	 * displayed at end of trip (= when stopped moving) ?
//   * when to reset?
// * OTA: load firmware from github release

#include <stdio.h>
#include <time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "json_settings.h"
#include "velo.h"

static const char *T = "MAIN";

static void velo_task(void *args)
{
	velogen_init();
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while (true) {
		velogen_loop();

		// Run with a fixed 20 Hz cycle rate
		vTaskDelayUntil(&xLastWakeTime, CYCLE_MS / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void app_main()
{
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
		esp_get_minimum_free_heap_size()
	);

	// Mount spiffs for *.html and defaults.json
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 4,
		.format_if_mount_failed = true
	};
    esp_vfs_spiffs_register(&conf);

	// Load settings.json from SPIFFS, try to create file if it doesn't exist
	set_settings_file("/spiffs/settings.json", "/spiffs/default_settings.json");

	// xTaskCreatePinnedToCore(velo_task, "velo_task", 4096, NULL, 1, NULL, 1);
	velo_task(NULL);
}
