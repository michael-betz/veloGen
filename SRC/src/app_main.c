// next steps
// * shutdown state machine (look for wifi, try upload)
// * per trip screens
//   * energy / charge / distance / avg speed
// 	 * displayed at end of trip (= when stopped moving) ?
//   * when to reset?
// * OTA: load firmware from github release

#include <stdio.h>
#include <time.h>
#include "esp32/rom/rtc.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "json_settings.h"
#include "velo.h"

static const char *T = "MAIN";

void app_main()
{
	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set("MQTT_CACHE", ESP_LOG_DEBUG);

	// report status
	ESP_LOGI(T,
		"reset reason: %d, heap: %d, min_heap: %d",
		rtc_get_reset_reason(0),
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

	velogen_init();

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while (true) {
		velogen_loop();

		// Run with a fixed 20 Hz cycle rate
		vTaskDelayUntil(&xLastWakeTime, 50 / portTICK_PERIOD_MS);
	}
}
