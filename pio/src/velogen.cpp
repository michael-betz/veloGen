#include "Arduino.h"
// #include "mqtt_client.h"
#include "driver/pcnt.h"
#include "Wire.h"
#include "ssd1306.h"
#include "ina219.h"
#include "json_settings.h"
#include "velo_wifi.h"
#include "velogen_gui.h"
#include "velogen.h"

#define N_PINS 4

static unsigned sleepTimeout = 30000;

// Number of wheel rotations since power up
RTC_DATA_ATTR unsigned g_wheelCnt;

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
	gpio_set_pull_mode(P_AC, GPIO_FLOATING);
	// Configure and enable the input filter
	// TODO fine tune filter to remove glitches at very low speed
	pcnt_set_filter_value(PCNT_UNIT_0, 500);
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
	unsigned ts = millis();
	int16_t pCnt = 0;
	if(pcnt_get_counter_value(PCNT_UNIT_0, &pCnt) == ESP_OK) {
		diffCnt = pCnt - lastCnt;
		g_wheelCnt += diffCnt;
		lastCnt = pCnt;
	}

	// TODO convert this calculation and IIR filter to fixed point integer
	// [milli counts / milli second] = [counts / second]
	float dC_dT = diffCnt * 1000.0 / (float)(ts - ts_);
	dC_dT = dC_dT * (float)um_p_pulse * 36.0 / 10000000.0;  // [km / hour]
	g_speed += 0.05 * (dC_dT - g_speed);

	ts_ = ts;
	return diffCnt;
}

static void touch_init()
{
	touch_threshold = jGetI(getSettings(), "touch_threshold", 12);

	// Arduino API is broken, use IDF one
	touch_pad_init();
	touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V5);
	touch_pad_set_meas_time(0x3000, 0x3000);
	for (int i=0; i<N_PINS; i++) {
		touch_pad_config(tPins[i], 0);
		touch_pad_read(tPins[i], &tinit[i]);
	}
}

void velogen_sleep()
{
	// Switch off OLED, shunt and dynamo
	inaOff();
	ssd_poweroff();
	digitalWrite(P_DYN, 0);

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
	esp_sleep_enable_ext1_wakeup((1 << P_AC), ESP_EXT1_WAKEUP_ALL_LOW);

	// ZzzZZZzzzZZ
	esp_deep_sleep_start();
}

// if a button has been released, sets its bit in return value
unsigned touch_read()
{
	static unsigned state_ = 0;
	unsigned state=0, release=0;

	for (int i=0; i<N_PINS; i++) {
		uint16_t tmp;
		touch_pad_read(tPins[i], &tmp);
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

// esp_mqtt_client_handle_t client;

// static int mqtt_event_handler(esp_mqtt_event_handle_t event)
// {
//     int msg_id;
//     // your_context_t *context = event->context;
//     switch (event->event_id) {
//         case MQTT_EVENT_CONNECTED:
//             log_i("MQTT_EVENT_CONNECTED");
//             msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
//             log_i("sent publish successful, msg_id=%d", msg_id);

//             msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
//             log_i("sent subscribe successful, msg_id=%d", msg_id);

//             msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
//             log_i("sent subscribe successful, msg_id=%d", msg_id);

//             msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
//             log_i("sent unsubscribe successful, msg_id=%d", msg_id);
//             break;
//         case MQTT_EVENT_DISCONNECTED:
//             log_i("MQTT_EVENT_DISCONNECTED");
//             break;

//         case MQTT_EVENT_SUBSCRIBED:
//             log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//             msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
//             log_i("sent publish successful, msg_id=%d", msg_id);
//             break;
//         case MQTT_EVENT_UNSUBSCRIBED:
//             log_i("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
//             break;
//         case MQTT_EVENT_PUBLISHED:
//             log_i("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
//             break;
//         case MQTT_EVENT_DATA:
//             log_i("MQTT_EVENT_DATA");
//             log_i("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//             log_i("DATA=%.*s\r\n", event->data_len, event->data);
//             break;
//         case MQTT_EVENT_ERROR:
//             log_i("MQTT_EVENT_ERROR");
//             break;
//         default:
//             log_i("Other event id:%d", event->event_id);
//             break;
//     }
//     return 0;
// }

void velogen_init()
{
	pinMode(P_DYN, OUTPUT);
	pinMode(P_AC, INPUT);
	digitalWrite(P_DYN, 0);

	counter_init();

	// init oled
	Wire.begin(12, 14, 800000);
	ssd_init();
	ssd_contrast(0);

	// init shunt
	inaInit();
	inaBus32(false);
	inaPga(0);
	inaAvg(7);

	touch_init();

	// Set the timezone
	setenv("TZ", jGetS(getSettings(), "timezone", "PST8PDT"), 1);
	tzset();

	sleepTimeout = jGetI(getSettings(), "sleep_timeout", 30) * 1000;
	initVeloWifi();
	tryConnect();

	// esp_mqtt_client_config_t mqtt_cfg;
	// mqtt_cfg.uri = "mqtt://roesti";
	// mqtt_cfg.event_handle = mqtt_event_handler;
	// client = esp_mqtt_client_init(&mqtt_cfg);
 //    esp_mqtt_client_start(client);
}

void velogen_loop()
{
	unsigned curTs = millis();
	static unsigned lastTick = 0;  // last TS when wheel moved / button pushed

	if (draw_screen())
		lastTick = curTs;

	if (counter_read())
		lastTick = curTs;

	if ((curTs - lastTick) > sleepTimeout)
		velogen_sleep();
}
