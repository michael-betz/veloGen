#include <string.h>
#include <stdio.h>
#include <time.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "gui.h"
#include "lv_font.h"
#include "ssd1306.h"
#include "ina219.h"
#include "velo_wifi.h"
#include "velo.h"
#include "velo_gui.h"

extern lv_font_t noto_sans_12;
extern lv_font_t concert_one_50;

#define N_SCREENS 4

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
	char buff[32];
	static t_label big_lbl;
	static const char * const units[] = {"km/h", "mA", "mV", "mW"};

	if (isInit) {
		// Static content which will not be refreshed
		lv_init_label(&big_lbl, 127, 0, 0, &noto_sans_12, units[type], LV_RIGHT);

		// Setup the bounding box of dynamic labels
		lv_init_label(&big_lbl, 64, 18, 0, &concert_one_50, "22222", LV_CENTER);
	}

	// clears and prints dynamic content into BB
	switch (type) {
		case 0:
			snprintf(buff, sizeof(buff), "%.1f", g_speed);
			break;

		case 1:
			snprintf(buff, sizeof(buff), "%d", g_mAmps);
			break;

		case 2:
			snprintf(buff, sizeof(buff), "%d", g_mVolts);
			break;

		case 3:
			snprintf(buff, sizeof(buff), "%d", g_mVolts * g_mAmps / 1000);
			break;
	}

	lv_update_label(&big_lbl, buff);
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
	static bool isInit = true, isDyn=false;

	// corresponding bit is 1 on button released
	unsigned btns = button_read();

	// left button: switch screen
	if (btns & (1 << 0)) {
		scr_id--;
		if (scr_id < 0)
			scr_id = 0;
	}

	// toggle dynamo
	if (btns & (1 << 1)) {
		isDyn = !isDyn;
		gpio_set_level(P_DYN, isDyn);
		setStatus(isDyn ? "Dyn ON!" : "Dyn off");
	}

	// toggle wifi
	if (btns & (1 << 2))
		toggle_wifi();

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

	switch (scr_id) {
		case 0:
		case 1:
		case 2:
		case 3:
			big_num(isInit, scr_id, btns);
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
