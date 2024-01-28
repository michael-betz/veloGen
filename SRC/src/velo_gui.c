#include <string.h>
#include <stdio.h>
#include <time.h>
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_netif.h"
#include "json_settings.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "gui.h"
#include "lv_font.h"
#include "ssd1306.h"
#include "ina219.h"
#include "velo_wifi.h"
#include "mqtt_cache.h"
#include "velo.h"
#include "static_ws.h"
#include "velo_gui.h"

#define N_SCREENS 5

extern const char ca_cert_start[] asm("_binary_ota_ca_cert_pem_start");

static const char *T = "VELO_GUI";

// Label for the status bar
static t_label stat_lbl = {
	.x = 0,
	.y = 0,
	.x0 = 0,
	.y0 = 0,
	.x1 = 98,
	.y1 = 15,
	.align = LV_LEFT,
	.fnt = &noto_sans_12
};

// screen layout: big number + unit on top left
static void big_num(bool isInit, int type, unsigned btns)
{
	static t_label big_lbl;
	static const char * const units[] = {"km/h", "mA", "V", "W"};
	static const char * const symbols[] = {SYM_TACHO, SYM_PLUG, SYM_CAR_BATTERY, SYM_BOLT};

	// toggle dynamo
	if (btns & (1 << 1))
		setDynamo(-1);

	// toggle wifi
	if (btns & (1 << 2))
		toggle_wifi();

	if (isInit) {
		// Static content which will not be refreshed
		lv_init_label(&big_lbl, 127, 0, &noto_sans_12, units[type], LV_RIGHT);
		lv_init_label(&big_lbl, 0, 23, &fa, symbols[type], LV_LEFT);

		// Setup the bounding box of dynamic labels
		lv_init_label(&big_lbl, 127, 19, &concert_one, "222", LV_RIGHT);
		big_lbl.x0 = 40;
	}

	// clears and prints dynamic content into BB
	switch (type) {
		case 0:
			lv_update_label(&big_lbl, "%d", g_speed / 10);
			break;

		case 1:
			lv_update_label(&big_lbl, "%d", g_mAmps);
			break;

		case 2:
			lv_update_label(&big_lbl, "%.2f", (float)g_mVolts / 1000);
			break;

		case 3:
			lv_update_label(&big_lbl, "%.1f", (float)g_mVolts * (float)g_mAmps / 1000.0 / 1000.0);
			break;
	}
}

static void ota_screen(bool isInit, int type, unsigned btns)
{
	static t_label ota_lbl;
	static bool doUpdate=false;
	static esp_err_t ret = -1;
	if (isInit) {
		if (isConnect) {
			startWebServer();
			lv_init_label(&ota_lbl, 63, 16, &noto_sans_12, "http://000.000.000.000", LV_CENTER);

			// print IP address.
			esp_netif_ip_info_t ip_info;
			esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
			char ip_addr[16];
			inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
			lv_update_label(&ota_lbl, "http://%s", ip_addr);
		}
		lv_init_label(&ota_lbl, 63, 32, &noto_sans_12, GIT, LV_CENTER);
		lv_init_label(&ota_lbl, 63, 48, &noto_sans_12, "    push 2 for OTA    ", LV_CENTER);
		doUpdate=false;
		ret = -1;
	}
	if (doUpdate) {
		esp_http_client_config_t hconfig = {
			.url = jGetS(getSettings(), "ota_url", ""),
			.cert_pem = ca_cert_start,
			// FIXME disables security, for development only
			.skip_cert_common_name_check = true
		};
		esp_https_ota_config_t config = {
			.http_config = &hconfig,
			.bulk_flash_erase = true,
			.partial_http_download = false,
			.max_http_request_size = 0,

		};
		ret = esp_https_ota(&config);
		log_w("ret: %d", ret);
		if (ret == ESP_OK)
			lv_update_label(&ota_lbl, "success! reboot?");
		else
			lv_update_label(&ota_lbl, "ret: %d", ret);
		doUpdate = false;
	}
	if (btns & (1 << 1)) {
		if (ret == ESP_OK)
			velogen_sleep(true);
		doUpdate = true;
		lv_update_label(&ota_lbl, "running OTA ...");
	}
}

static char status_text[24];
unsigned status_update_ts = 0;

// called in the main loop, reads buttons,
// chooses which screen to initialize / refresh
// returns button states
unsigned draw_screen()
{
	static unsigned frm=0;
	static int scr_id = 0;
	static bool isInit = true;

	// corresponding bit is 1 on button released
	unsigned btns = button_read();

	// left button: switch screen
	if (btns & (1 << 0)) {
		scr_id--;
		if (scr_id < 0)
			scr_id = 0;
	}

	// right button: switch screen
	if (btns & (1 << 3)) {
		scr_id++;
		if (scr_id >= N_SCREENS)
			scr_id = N_SCREENS - 1;
	}

	// check if screen needs to be re-drawn completely
	if (isInit || btns & 0x09) {
		isInit = true;
		fill(0);
		lv_update_label(&stat_lbl, status_text);

		if (scr_id != 4)
			stopWebServer();
	}

	// draw clock if no new status message for 10 s
	if ((frm % 20) == 0 && (xTaskGetTickCount() - status_update_ts) > (10000 / portTICK_PERIOD_MS)) {
		time_t now = time(NULL);
		struct tm timeinfo = {0};
		localtime_r(&now, &timeinfo);
		if (timeinfo.tm_sec == 0) {
			strftime(status_text, sizeof(status_text), "%d.%m.  %H:%M", &timeinfo);
			lv_update_label(&stat_lbl, status_text);
		}
	}

	// button action common to many screens
	switch (scr_id) {
		case 0:
		case 1:
		case 2:
		case 3:
			big_num(isInit, scr_id, btns);
			break;

		case 4:
			ota_screen(isInit, scr_id, btns);
			break;
	}
	// Send framebuffer to OLED
	ssd_send();
	isInit = false;
	frm++;
	return btns;
}

// changes status line on oled, call like printf (how cool is that!)
void setStatus(const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(status_text, sizeof(status_text), format, argptr);
	va_end(argptr);

	status_update_ts = xTaskGetTickCount();
	lv_update_label(&stat_lbl, status_text);
}
