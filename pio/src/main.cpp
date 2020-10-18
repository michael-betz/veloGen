// next steps
//   * count zero crossings
//   * wifi manager (connect to knwon wifis / AP mode)
//   * power manager (sleep when not moving)
//   * check reset cause: brown out -- dont enable wifi
//   * gui status bar
//   * upload stats (mqtt?)

#include <stdio.h>
#include "Arduino.h"
#include "ArduinoOTA.h"
#include "SPIFFS.h"
#include "rom/rtc.h"
#include "json_settings.h"
#include "velogen.h"

void setup()
{
	// This freezes the CPU after wake up from deep sleep :(
	// setCpuFrequencyMhz(80);

	// report status
	log_w(
		"reset reason: %d, heap: %d, min_heap: %d",
		rtc_get_reset_reason(0),
		esp_get_free_heap_size(),
		esp_get_minimum_free_heap_size()
	);

	// Mount spiffs for *.html and defaults.json
	SPIFFS.begin(true, "/spiffs", 4);

	// Load settings.json from SPIFFS, try to create file if it doesn't exist
	set_settings_file("/spiffs/settings.json", "/spiffs/default_settings.json");

	velogen_init();

	int frm=0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while (true) {
		velogen_loop();

		if ((frm % 100) == 0) {
			time_t now = time(NULL);
			struct tm timeinfo = {0};
			char strftime_buf[64];
			localtime_r(&now, &timeinfo);
			strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
			log_i("Local Time: %s (%ld)", strftime_buf, now);
		}

		ArduinoOTA.handle();
		frm++;
		// Run with a fixed 20 Hz cycle rate
		vTaskDelayUntil(&xLastWakeTime, 50 / portTICK_PERIOD_MS);
	}
}

void loop() {
	vTaskDelete(NULL);
}
