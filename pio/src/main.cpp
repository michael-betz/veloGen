// demonstrates the use of esp-comms
#include <stdio.h>
#include "Arduino.h"
// #include "ArduinoOTA.h"
#include "SPIFFS.h"
#include "rom/rtc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "esp_comms.h"
// #include "web_console.h"
// #include "json_settings.h"

#include "Wire.h"
#include "ssd1306.h"
#include "ina219.h"

#include "lv_font.h"
#include "velogen_gui.h"

// Web socket RX data received callback
// static void on_ws_data(
// 	AsyncWebSocket * server,
// 	AsyncWebSocketClient * client,
// 	AwsEventType type,
// 	void * arg,
// 	uint8_t *data,
// 	size_t len
// ) {
// 	switch (data[0]) {
// 		case 'a':
// 			wsDumpRtc(client);  // read rolling log buffer in RTC memory
// 			break;

// 		case 'b':
// 			settings_ws_handler(client, data, len);  // read / write settings.json
// 			break;

// 		case 'r':
// 			ESP.restart();
// 			break;

// 		case 'h':
// 			client->printf(
// 				"h{\"heap\": %d, \"min_heap\": %d}",
// 				esp_get_free_heap_size(), esp_get_minimum_free_heap_size()
// 			);
// 			break;
// 	}
// }

static const int tPins[] = {0, 2, 4, 32};
volatile unsigned touchflags = 0;

void setup()
{
	//------------------------------
	// init stuff
	//------------------------------
	// forward serial characters to web-console
	// web_console_init();

	// report status
	log_w(
		"reset reason: %d, heap: %d, min_heap: %d",
		rtc_get_reset_reason(0),
		esp_get_free_heap_size(),
		esp_get_minimum_free_heap_size()
	);

	// Mount spiffs for *.html and defaults.json
	// SPIFFS.begin(true, "/spiffs", 4);

	// Load settings.json from SPIFFS, try to create file if it doesn't exist
	// set_settings_file("/spiffs/settings.json", "/spiffs/default_settings.json");

	// init web-server
	// init_comms(SPIFFS, "/", on_ws_data);
	// log_w(
	// 	"after COMMS, heap: %d, min_heap: %d",
	// 	esp_get_free_heap_size(),
	// 	esp_get_minimum_free_heap_size()
	// );

	// init oled
	Wire.begin(12, 14, 800000);
	ssd_init();

	inaInit();
	inaBus32(false);
	inaPga(0);
	inaAvg(7);

	touchSetCycles(0x4000, 0x4000);
	int thrs[4];
	for (int i=0; i<4; i++)
		thrs[i] = touchRead(tPins[i]) - 15;
	touchAttachInterrupt(tPins[0], []() {touchflags |= 1 << 0;}, thrs[0]);
	touchAttachInterrupt(tPins[1], []() {touchflags |= 1 << 1;}, thrs[1]);
	touchAttachInterrupt(tPins[2], []() {touchflags |= 1 << 2;}, thrs[2]);
	touchAttachInterrupt(tPins[3], []() {touchflags |= 1 << 3;}, thrs[3]);
}

void loop() {
	static unsigned frm=0;

	if (touchflags) {
		log_i("touch: %x", touchflags);
		touchflags = 0;
	}

	draw_screen((frm / 100) % 4);

	// ArduinoOTA.handle();
	delay(50);
	frm++;
}
