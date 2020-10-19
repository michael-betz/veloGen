#include <string.h>
#include "json_settings.h"
#include "lwip/apps/sntp.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
// #include "static_ws.h"
#include "mqtt_client.h"
#include "velo.h"
#include "velo_gui.h"

static const char *T = "VELO_WIFI";

bool isConnect = false;
esp_mqtt_client_handle_t mqtt_c;


// go through scan results and look for the first known wifi
static void scan_done(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	uint16_t n = 24;
	static wifi_ap_record_t ap_info[24];

	E(esp_wifi_scan_get_ap_records(&n, ap_info));

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
		const char *ssid = (const char *)ap_info[i].ssid;
		log_d("%s, %d, %d", ssid, ap_info[i].primary, ap_info[i].rssi);
		jWifi = jGet(jWifis, ssid);
		if (!cJSON_IsString(jWifi))
			continue;
		char *pw = jWifi->valuestring;

		// Found a known good WIFI, connect to it ...
		wifi_config_t cfg;
		memset(&cfg, 0, sizeof(cfg));
		strncpy((char*)cfg.sta.ssid, ssid, 32);
		strncpy((char*)cfg.sta.password, pw, 64);
		cfg.sta.scan_method = WIFI_FAST_SCAN;
		cfg.sta.bssid_set = true;
		memcpy(cfg.sta.bssid, ap_info[i].bssid, 6);
		cfg.sta.channel = ap_info[i].primary;
		cfg.sta.pmf_cfg.capable = true;
		log_i("Looks familiar: %s", cfg.sta.ssid);

		E(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));
		E(esp_wifi_connect());
		E(esp_wifi_set_ps(WIFI_PS_MAX_MODEM));
	    E(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, jGetS(getSettings(), "hostname", WIFI_HOST_NAME)));
		setStatus(ssid);
		return;
	}
	log_i("no known wifi found");
	setStatus("No wifi");
	esp_wifi_stop();
}

static void got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	log_i("got an IP. NICE!!");

	// trigger time sync
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, (char *)"pool.ntp.org");
	sntp_init();

	// esp_mqtt_client_reconnect(mqtt_c);
	esp_mqtt_client_start(mqtt_c);
}

static void got_discon(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	log_w("got disconnected :(");
	if (event_data) {
		wifi_event_sta_disconnected_t *ed = (wifi_event_sta_disconnected_t*)event_data;
		log_w("reason: %d", ed->reason);
	}

	setStatus("No wifi");
	sntp_stop();
	esp_wifi_stop();
	esp_mqtt_client_stop(mqtt_c);
	// esp_mqtt_client_disconnect(mqtt_c);
	isConnect = false;
}

static int mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			log_i("MQTT_EVENT_CONNECTED");
			break;
		case MQTT_EVENT_DISCONNECTED:
			log_i("MQTT_EVENT_DISCONNECTED");
			break;
		case MQTT_EVENT_SUBSCRIBED:
			log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_UNSUBSCRIBED:
			log_i("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_PUBLISHED:
			// log_i("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			log_i("MQTT_EVENT_DATA");
			log_i("TOPIC=%.*s\r\n", event->topic_len, event->topic);
			log_i("DATA=%.*s\r\n", event->data_len, event->data);
			break;
		case MQTT_EVENT_ERROR:
			log_e("MQTT_EVENT_ERROR");
			break;
		default:
			log_i("Other event id:%d", event->event_id);
			break;
	}
	return 0;
}

extern const char DST_Root_CA_X3_pem[] asm("_binary_DST_Root_CA_X3_pem_start");
extern const char DST_Root_CA_X3_pem_e[] asm("_binary_DST_Root_CA_X3_pem_end");

void initVeloWifi()
{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		E(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	E(ret);

	E(esp_netif_init());
	E(esp_event_loop_create_default());
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	E(esp_wifi_init(&cfg));

	// E(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	//  register some async callbacks
	E(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &scan_done, NULL));
	E(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip, NULL));
	E(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &got_discon, NULL));
	E(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_STOP, &got_discon, NULL));

	// Initialize default station as network interface instance (esp-netif)
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);

	// No websocket server support, so what's the point :(
	// startWebServer();

	// MQTT client
	esp_mqtt_client_config_t mqtt_cfg;
	memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));
	mqtt_cfg.uri = jGetS(getSettings(), "mqtt_url", "mqtt://roesti");
	// log_i("MQTT broker @ %s", mqtt_cfg.uri);  // careful, URL may contain PW

	//_DST_Root_CA_X3.pem to verify server public keys are legit
	// copy of /etc/ssl/certs/DST_Root_CA_X3.pem
	// matching broker configuration:
	// https://www.digitalocean.com/community/tutorials/how-to-install-and-secure-the-mosquitto-mqtt-messaging-broker-on-ubuntu-18-04-quickstart
	mqtt_cfg.cert_pem = DST_Root_CA_X3_pem;
	mqtt_cfg.cert_len = DST_Root_CA_X3_pem_e - DST_Root_CA_X3_pem;

	mqtt_cfg.event_handle = mqtt_event_handler;
	mqtt_cfg.disable_auto_reconnect = true;
	mqtt_c = esp_mqtt_client_init(&mqtt_cfg);
}

void tryConnect()
{
	isConnect = true;
	setStatus("Scanning ...");
	// Initialize and start WiFi scan
	E(esp_wifi_set_mode(WIFI_MODE_STA));
	E(esp_wifi_start());
	E(esp_wifi_scan_start(NULL, false));
	// fires SYSTEM_EVENT_SCAN_DONE when done, calls scan_done() ...
}

void toggle_wifi()
{
	if (isConnect)
		esp_wifi_disconnect();
	else
		tryConnect();
}
