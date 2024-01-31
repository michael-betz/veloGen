// #include "mqtt_client.h"
#include <errno.h>
#include <string.h>
#include <math.h>
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "driver/pulse_cnt.h"
#include "driver/touch_pad.h"
#include "driver/i2c.h"
#include "driver/rtc_io.h"
#include "mqtt_client.h"
#include "time.h"
#include "ws2812.h"
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

int g_speed = 0;  // [km * 10 / h]

// settings from the .json file
// wheel circumference = 2155 mm
// pulses / revolution = 13
// distance / pulse = 165769 um
static unsigned um_p_pulse=0;
static int touch_threshold=0;

// val: -1: toggle, 0: Off, 1: On
void setDynamo(int val)
{
	static bool isDyn = false;
	bool isDyn_ = isDyn;
	if (val == -1)
		isDyn = !isDyn;
	else
		isDyn = val > 0;
	gpio_set_level(P_DYN, isDyn);
	if (isDyn != isDyn_)
		setStatus(isDyn ? "Dyn ON!" : "Dyn off");
}

pcnt_unit_handle_t pcnt_unit = NULL;

// Pulse counter to count wheel rotations
static void counter_init()
{
	um_p_pulse = jGetI(getSettings(), "um_p_pulse", 165769);

	pcnt_unit_config_t unit_config = {
		.high_limit = 0x7FFF,
		.low_limit = -1,
	};
	ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

	pcnt_chan_config_t chan_config = {
		.edge_gpio_num = P_AC,
		.level_gpio_num = -1,
	};
	pcnt_channel_handle_t pcnt_chan = NULL;
	ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan));

	// decrease the counter on rising edge
	ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
	// ignore control signal
	ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

	pcnt_glitch_filter_config_t filter_config = {
		.max_glitch_ns = 10000,
	};
	ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));
	ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
	ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
	gpio_set_pull_mode(P_AC, GPIO_FLOATING);
}

// Moving average over 2**MA_WIDTH values
#define MA_WIDTH 5

// convert [um / MA_TIME] to [km * 10 / h]
#define CONV_CONST (CYCLE_MS * (1<<MA_WIDTH) * 277778ll / 10000)

unsigned ma(unsigned val)
{
	#define MA_MASK ((1 << MA_WIDTH) - 1)
	static unsigned mas[1 << MA_WIDTH], wp=0;

	mas[wp] = val;
	wp = (wp + 1) & MA_MASK;

	unsigned sum = 0;
	for (unsigned i=0; i<(1 << MA_WIDTH); i++)
		sum += mas[i];
	return sum;
}

unsigned counter_read()
{
	int diffCnt=0;
	unsigned diffCnt_avg=0;
	static int lastCnt = 0;
	// static unsigned ts_ = 0;
	// unsigned ts = xTaskGetTickCount();
	int pCnt = 0;
	if(pcnt_unit_get_count(pcnt_unit, &pCnt) != ESP_OK) {
		log_e("Failed reading counter :(");
		return 0;
	}

	diffCnt = pCnt - lastCnt;
	lastCnt = pCnt;
	if (diffCnt < 0 || diffCnt > 50) {
		log_e("Bad diffCnt: %d", diffCnt);
		return 0;
	}
	g_wheelCnt += diffCnt;

	// TODO convert this calculation and IIR filter to fixed point integer
	// [milli counts / milli second] = [counts / second]
	// float dC_dT = diffCnt * 1000.0 / 50e-3;  // fixed 50 ms cycle rate
	// dC_dT = dC_dT * (float)um_p_pulse * 36.0 / 10000000.0;  // [km / hour]

	// example values at 40 km/h, MA_WIDTH = 6
	// diffCnt: [pulses / 50 ms]: 3.35
	// diffCnt_avg  [pulses / 3200 ms]: 214.4
	// diffCnt_avg * um_p_pulse  [um / 3200 ms]: 35540873.6

	diffCnt_avg = ma(diffCnt);

	// [km * 10 / h]
	g_speed = (diffCnt_avg * um_p_pulse + (CONV_CONST / 2)) / CONV_CONST;

	// if (!isnormal(g_speed))
	// 	g_speed = 0.0;
	// g_speed += 0.03 * (dC_dT - g_speed);

	// ts_ = ts;
	return diffCnt;
}

static bool flip_buttons = false;

static void touch_init()
{
	cJSON *s = getSettings();
	touch_threshold = jGetI(s, "touch_threshold", 30);
	flip_buttons = jGetB(s, "oled_rotate_180", false);

	// Arduino API is broken, use IDF one
	touch_pad_init();
	touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V5);
	touch_pad_set_measurement_interval(0x3000);
	touch_pad_set_measurement_clock_cycles(0x3000);
	for (int i=0; i<N_PINS; i++) {
		touch_pad_config(tPins[i], 0);
		touch_pad_read(tPins[i], &tinit[i]);
	}
}

void velogen_sleep(bool isReboot)
{
	ws2812_off();
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
	gpio_set_level(P_5V, 0);

	// disable aux power pins
	gpio_set_level(P_EN1, 0);
	gpio_set_level(P_EN2, 0);

	// Initialize touch pad peripheral for FSM timer mode
	touch_pad_init();
	touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
	// Set reference voltage for charging/discharging
	touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V5);
	touch_pad_set_measurement_interval(0xFFFF);
	touch_pad_set_measurement_clock_cycles(0x3000);
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

	// get capacitance values and subtract offset to normalize them
	for (int i=0; i<N_PINS; i++) {
		uint16_t tmp;
		touch_pad_read(tPins[i], &tmp);
		tpv[i] = tinit[i] - tmp;
	}

	// figure out which buttons have been released, flip buttons if needed
	for (int i=0; i<N_PINS; i++)
		if (tpv[flip_buttons ? N_PINS - 1 - i : i] > touch_threshold)
			state |= 1 << i;
		else if ((state_ >> i) & 1)
			release |= 1 << i;

	// if (state)
	//     log_i("TP: %3d %3d %3d %3d", tpv[0], tpv[1], tpv[2], tpv[3]);

	state_ = state;

	return release;
}

static int g_ws2812_intensity = 0x80;

void velogen_init()
{
	gpio_set_direction(P_DYN, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_direction(P_5V, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_direction(P_EN1, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_direction(P_EN2, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_direction(P_AC, GPIO_MODE_INPUT);
	setDynamo(1);
	gpio_set_level(P_5V, 0);

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
	tryConnect();
	cache_init();  // open / create cache file on SPIFFS

	// init led strip last, so power can stabilize
	ws2812_init();
	g_ws2812_intensity = jGetI(s, "strip_intensity", 0x80);
}

bool g_is_lights = false;

// Switch on / off the lights and dynamo
void power_house_keeping()
{
	static int last_volts = -1;
	static const int max_volts = 8500;

	// crude battery protection
	if (g_mVolts > max_volts)
		setDynamo(0);
	else if (g_mVolts < (max_volts - 200) && last_volts >= (max_volts - 200))
		setDynamo(1);

	time_t now = time(NULL);
	struct tm timeinfo = {0};
	localtime_r(&now, &timeinfo);
	int hour = timeinfo.tm_hour;

	if (hour > 9 && hour < 17) {
		if (g_is_lights) {
			g_is_lights = false;
			ws2812_off();
			gpio_set_level(P_5V, 0);
			gpio_set_level(P_EN1, 0);
			gpio_set_level(P_EN2, 0);
			setStatus("Lights off!");
		}
	} else {
		if (!g_is_lights) {
			g_is_lights = true;
			gpio_set_level(P_5V, 1);
			gpio_set_level(P_EN1, 1);
			gpio_set_level(P_EN2, 1);
			setStatus("Lights ON!");
		}
	}

	last_volts = g_mVolts;
}

// main loop, called precisely every 50 ms
void velogen_loop()
{
	static int frm = 0;

	int curTs = xTaskGetTickCount();
	static int ts_sleep=0;
	static int ts_con=300000 / portTICK_PERIOD_MS;  // last TS when wheel moved / wanted to connect

	g_mVolts = inaV();
	g_mAmps = inaI();

	if ((frm % 50) == 0)
		power_house_keeping();

	if (counter_read()) {
		// If wheel was moved
		ts_sleep = curTs;
		ts_con = curTs;
	}

	// 20 Hz max.
	cache_handle();

	if (draw_screen())
		ts_sleep = curTs;

	if ((curTs - ts_sleep) > sleepTimeout)
		velogen_sleep(false);

	if (g_is_lights)
		ws2812_animate(g_ws2812_intensity);

	// we stopped, try to connect to wifi after 10s
	if (((curTs - ts_con) > (10000 / (int)portTICK_PERIOD_MS)) && !isConnect) {
		tryConnect();
		// don't try to re-connect in the next 5 minutes
		ts_con += sleepTimeout;
	}

	frm++;
}
