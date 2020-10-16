#include "Arduino.h"

#include "SPIFFS.h"
#include "json_settings.h"
#include "esp_comms.h"
#include "web_console.h"
#include "lwip/apps/sntp.h"

#include "velogen.h"
#include "velogen_gui.h"


// Web socket RX data received callback
static void on_ws_data(
	AsyncWebSocket * server,
	AsyncWebSocketClient * client,
	AwsEventType type,
	void * arg,
	uint8_t *data,
	size_t len
) {
	switch (data[0]) {
		case 'a':
			wsDumpRtc(client);  // read rolling log buffer in RTC memory
			break;

		case 'b':
			settings_ws_handler(client, data, len);  // read / write settings.json
			break;

		case 'r':
			ESP.restart();
			break;

		case 'h':
			client->printf(
				"h{\"heap\": %d, \"min_heap\": %d}",
				esp_get_free_heap_size(), esp_get_minimum_free_heap_size()
			);
			break;
	}
}

// go through scan results and look for the first known wifi
static void scan_done(WiFiEvent_t event, WiFiEventInfo_t info)
{
	int n = WiFi.scanComplete();
	if (n < 1) {
		log_e("wifi scan failed");
		return;
	}
	cJSON *jWifi, *jWifis = jGet(getSettings(), "wifis");
	if (!jWifis) {
		log_e("no wifis defined in .json");
		return;
	}
	// Go through all found APs, use ssid as key and try to get item from json
	for (unsigned i=0; i<n; i++) {
		const char *ssid = WiFi.SSID(i).c_str();
		jWifi = jGet(jWifis, ssid);
		if (!cJSON_IsString(jWifi))
			continue;

		// Found a known good WIFI, connect to it ...
		char *pw = jWifi->valuestring;
		log_i("Looks familiar: %s", ssid);
		setStatus(ssid);
    	WiFi.begin(ssid, pw);
		return;
	}
	log_i("no known wifi found");
    setStatus("No wifi");
	WiFi.mode(WIFI_OFF);
}

static void got_ip(WiFiEvent_t event, WiFiEventInfo_t info)
{
	log_i("got an IP. NICE!!");

	// init web-server
	startServices(SPIFFS, "/", on_ws_data);

	g_sleepTimeout = 300000;
}

static void got_discon(WiFiEvent_t event, WiFiEventInfo_t info)
{
	log_i("got disconnected :(");
	setStatus("No wifi");
	stopServices();
	WiFi.mode(WIFI_OFF);
	g_sleepTimeout = 30000;
}

void initVeloWifi()
{
	//  register some async callbacks
    WiFi.onEvent(scan_done, SYSTEM_EVENT_SCAN_DONE);
    WiFi.onEvent(got_ip, SYSTEM_EVENT_STA_GOT_IP);
    WiFi.onEvent(got_discon, SYSTEM_EVENT_STA_DISCONNECTED);
    WiFi.onEvent(got_discon, SYSTEM_EVENT_STA_STOP);
}

void tryConnect()
{
    WiFi.disconnect();
	WiFi.mode(WIFI_STA);
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
    setStatus("Scanning ...");
    // fires SYSTEM_EVENT_SCAN_DONE when done, calls scan_done() ...
}
