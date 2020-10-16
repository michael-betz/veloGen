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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_comms.h"
#include "web_console.h"
#include "json_settings.h"

#include "Wire.h"
#include "ssd1306.h"
#include "ina219.h"
#include "velogen.h"

#include "lv_font.h"
#include "velogen_gui.h"

void setup()
{
	// This freezes the CPU after wake up from deep sleep :(
	// setCpuFrequencyMhz(80);

	//------------------------------
	// init network stuff
	//------------------------------
	// forward serial characters to web-console
	web_console_init();

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
}

void loop() {
	unsigned curTs = millis();
	static unsigned lastTick = 0;
	static int frm=0;

	if (draw_screen())
		lastTick = curTs;

	if (counter_read())
		lastTick = curTs;

	if ((curTs - lastTick) > g_sleepTimeout)
	{
		log_w("Going to sleep now");
		delay(1000);
		inaOff();
		ssd_poweroff();
		digitalWrite(P_DYN, 0);
		prepare_sleep();
  		esp_deep_sleep_start();
	}

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
	delay(50);
}
