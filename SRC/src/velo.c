// #include "mqtt_client.h"
#include <errno.h>
#include <string.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "driver/pcnt.h"
#include "driver/touch_pad.h"
#include "driver/i2c.h"
#include "driver/rtc_io.h"
#include "mqtt_client.h"
#include "time.h"
#include "ssd1306.h"
#include "ina219.h"
#include "json_settings.h"
#include "mqtt_cache.h"
#include "velo_wifi.h"
#include "velo_gui.h"
#include "static_ws.h"
#include "velo.h"

#define N_PINS 4

static const char *T = "VELOGEN";

static unsigned sleepTimeout = 30000;

// Number of wheel rotations since power up
RTC_DATA_ATTR unsigned g_wheelCnt;
int g_mVolts = 0;
int g_mAmps = 0;

// touch pin numbers
static const touch_pad_t tPins[N_PINS] = {
	TOUCH_PAD_NUM1, TOUCH_PAD_NUM2, TOUCH_PAD_NUM0, TOUCH_PAD_NUM9
};

// initial reading at boot (without touch)
static uint16_t tinit[N_PINS];

// Current delta values
static int tpv[N_PINS];

float g_speed=0;

// settings from the .json file
static int um_p_pulse=0, touch_threshold=0;

// Pulse counter to count wheel rotations
static void counter_init()
{
	// wheel circumference = 2155 mm
	// pulses / revolution = 13
	// distance / pulse = 165769 um
	um_p_pulse = jGetI(getSettings(), "um_p_pulse", 165769);

	pcnt_config_t pcnt_config = {
		// Set PCNT input signal and control GPIOs
		.pulse_gpio_num = P_AC,
		.ctrl_gpio_num = PCNT_PIN_NOT_USED,
		// What to do when control input is low or high?
		.lctrl_mode = PCNT_MODE_KEEP,  // Keep the primary counter mode if low
		.hctrl_mode = PCNT_MODE_KEEP,  // Keep the primary counter mode if high
		// What to do on the positive or negative edge of pulse input?
		.pos_mode = PCNT_COUNT_INC,  // Count up on the positive edge
		.neg_mode = PCNT_COUNT_DIS,  // Keep the counter value on the negative edge
		.counter_h_lim = 0x7FFF,
		.counter_l_lim = 0,
		.unit = PCNT_UNIT_0,
		.channel = PCNT_CHANNEL_0,
	};
	// Initialize PCNT unit
	pcnt_unit_config(&pcnt_config);
	rtc_gpio_deinit(P_AC);
	gpio_set_pull_mode(P_AC, GPIO_FLOATING);
	// Configure and enable the input filter
	pcnt_set_filter_value(PCNT_UNIT_0, 1023);
	pcnt_filter_enable(PCNT_UNIT_0);
	// Initialize PCNT's counter
	pcnt_counter_pause(PCNT_UNIT_0);
	pcnt_counter_clear(PCNT_UNIT_0);
	// Everything is set up, now go to counting
	pcnt_counter_resume(PCNT_UNIT_0);
}

int counter_read()
{
	int diffCnt = 0;
	static int16_t lastCnt = 0;
	static unsigned ts_ = 0;
	unsigned ts = xTaskGetTickCount();
	int16_t pCnt = 0;
	if(pcnt_get_counter_value(PCNT_UNIT_0, &pCnt) == ESP_OK) {
		diffCnt = pCnt - lastCnt;
		g_wheelCnt += diffCnt;
		lastCnt = pCnt;
	}

	// TODO convert this calculation and IIR filter to fixed point integer
	// [milli counts / milli second] = [counts / second]
	float dC_dT = diffCnt * 1000.0 / (float)(ts - ts_) / portTICK_PERIOD_MS;
	dC_dT = dC_dT * (float)um_p_pulse * 36.0 / 10000000.0;  // [km / hour]
	g_speed += 0.03 * (dC_dT - g_speed);

	ts_ = ts;
	return diffCnt;
}

static bool flip_buttons = false;

static void touch_init()
{
	cJSON *s = getSettings();
	touch_threshold = jGetI(s, "touch_threshold", 12);
	flip_buttons = jGetB(s, "oled_rotate_180", false);

	// Arduino API is broken, use IDF one
	touch_pad_init();
	touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V5);
	touch_pad_set_meas_time(0x3000, 0x3000);
	for (int i=0; i<N_PINS; i++) {
		touch_pad_config(tPins[i], 0);
		touch_pad_read(tPins[i], &tinit[i]);
	}
}

void velogen_sleep(bool isReboot)
{
	esp_wifi_disconnect();
	if (f_buf)
		fclose(f_buf);
	vTaskDelay(100 / portTICK_PERIOD_MS);

	if (isReboot) {
		log_e("calling esp_restart()");
		esp_restart();
	}

	// Switch off OLED, shunt and dynamo
	inaOff();
	ssd_poweroff();
	gpio_set_level(P_DYN, 0);

	// Initialize touch pad peripheral for FSM timer mode
	touch_pad_init();
	touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
	// Set reference voltage for charging/discharging
	touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V5);
	touch_pad_set_meas_time(0xFFFF, 0x3000);
	//init RTC IO and mode for touch pad.
	for (int i=0; i<N_PINS; i++) {
		touch_pad_config(tPins[i], tinit[i] - touch_threshold);
	}
	esp_sleep_enable_touchpad_wakeup();

	// enable wheel pulse as wakeup source
	esp_sleep_enable_ext1_wakeup((1 << P_AC), ESP_EXT1_WAKEUP_ANY_HIGH);

	esp_deep_sleep_start();  // ZzzZZZzzzZZ
}

// if a button has been released, sets its bit in return value
unsigned button_read()
{
	static unsigned state_ = 0;
	unsigned state=0, release=0;

	for (int i=0; i<N_PINS; i++) {
		uint16_t tmp;
		touch_pad_read(tPins[flip_buttons ? N_PINS - 1 - i : i], &tmp);
		tpv[i] = tinit[i] - tmp;
		if (tpv[i] > touch_threshold) {
			state |= 1 << i;
		} else if ((state_ >> i) & 1) {
			release |= 1 << i;
		}
	}

	// if (state)
	//     log_v("TP: %3d %3d %3d %3d", tpv[0], tpv[1], tpv[2], tpv[3]);

	state_ = state;

	return release;
}

void velogen_init()
{
	gpio_set_direction(P_DYN, GPIO_MODE_OUTPUT);
	gpio_set_direction(P_AC, GPIO_MODE_INPUT);
	gpio_set_level(P_DYN, 1);

	counter_init();

	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = 12,
		.scl_io_num = 14,
		.sda_pullup_en = false,
		.scl_pullup_en = false,
		.master.clk_speed = 800000  // overclocked x2
	};
	i2c_param_config(I2C_NUM_0, &conf);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	cJSON *s = getSettings();

	// init oled
	ssd_init();
	if (jGetB(s, "oled_rotate_180", false)) {
		ssd_flip_x(true);
		ssd_flip_y(true);
	}
	if (jGetB(s, "oled_rand_invert", false))
		ssd_invert(esp_random() & 1);
	fill(0);
	ssd_send();

	// init shunt
	inaInit();
	inaBus32(false);
	inaPga(0);
	inaAvg(7);

	touch_init();

	// Set the timezone
	setenv("TZ", jGetS(s, "timezone", "PST8PDT"), 1);
	tzset();

	sleepTimeout = jGetI(s, "sleep_timeout", 30) * 1000 / portTICK_PERIOD_MS;

	initVeloWifi();
	cache_init();  // open / create cache file on SPIFFS
	tryConnect();
}

// main loop, called precisely every 50 ms
void velogen_loop()
{
	static int frm = 0;

	int curTs = xTaskGetTickCount();
	static int ts_sleep=0;
	// static int ts_con=300000 / portTICK_PERIOD_MS;  // last TS when wheel moved / wanted to connect

	g_mVolts = inaV();
	g_mAmps = inaI();

	// crude battery protection
	if (g_mVolts > 8400)
		gpio_set_level(P_DYN, 0);

	if (counter_read()) {
		ts_sleep = curTs;
		// ts_con = curTs;
	}

	// 20 Hz max.
	cache_handle();

	if (draw_screen())
		ts_sleep = curTs;

	if ((curTs - ts_sleep) > sleepTimeout)
		velogen_sleep(false);

	// we stopped, try to connect to wifi after 10s
	// if (((curTs - ts_con) > (10000 / (int)portTICK_PERIOD_MS)) && !isConnect) {
	// 	tryConnect();
	// 	// don't try to re-connect in the next 5 minutes
	// 	ts_con += sleepTimeout;
	// }

	frm++;
}
