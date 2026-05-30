// #include "mqtt_client.h"
#include "velo.h"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "ina219.h"
#include "json_settings.h"
#include "main.h"
#include "mqtt_cache.h"
#include "mqtt_client.h"
#include "nmea_parser.h"
#include "static_ws.h"
#include "time.h"
#include "wifi.h"
#include "ws2812.h"
#include <time.h>

#define N_PINS 4

static const char *T = "VELOGEN";

static unsigned sleepTimeout = 30000;

// Number of wheel rotations since power up
RTC_DATA_ATTR unsigned g_wheelCnt;
int g_mVolts = 0;
int g_mAmps = 0;
int g_speed = 0;  // [km * 10 / h]

// settings from the .json file
// wheel circumference = 2155 mm
// pulses / revolution = 13
// distance / pulse = 165769 um
static unsigned um_p_pulse = 0;

// 0: Off, 1: On
void setAuxPower(bool val) { gpio_set_level(P_AUX_PWR, val); }

pcnt_unit_handle_t pcnt_unit = NULL;
nmea_parser_handle_t nmea_hdl = NULL;

// Pulse counter to count wheel rotations
static void counter_init() {
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
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD));
    // ignore control signal
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        pcnt_chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

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
#define CONV_CONST (CYCLE_MS * (1 << MA_WIDTH) * 277778ll / 10000)

unsigned ma(unsigned val) {
#define MA_MASK ((1 << MA_WIDTH) - 1)
    static unsigned mas[1 << MA_WIDTH], wp = 0;

    mas[wp] = val;
    wp = (wp + 1) & MA_MASK;

    unsigned sum = 0;
    for (unsigned i = 0; i < (1 << MA_WIDTH); i++)
        sum += mas[i];
    return sum;
}

unsigned counter_read() {
    int diffCnt = 0;
    unsigned diffCnt_avg = 0;
    static int lastCnt = 0;
    // static unsigned ts_ = 0;
    // unsigned ts = xTaskGetTickCount();
    int pCnt = 0;
    if (pcnt_unit_get_count(pcnt_unit, &pCnt) != ESP_OK) {
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

void velogen_sleep(bool isReboot) {
    ws2812_off();
    esp_wifi_disconnect();
    if (f_buf)
        fclose(f_buf);
    gps_sleep(nmea_hdl);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    if (isReboot) {
        log_e("calling esp_restart()");
        esp_restart();
    }

    // Switch off everything that my drain power
    inaOff();
    gpio_set_level(P_AUX_PWR, 0);

    // enable wheel pulse as wakeup source
    // esp_sleep_enable_ext1_wakeup((1 << P_AC), ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_sleep_enable_ext1_wakeup((1 << P_BOOT0), ESP_EXT1_WAKEUP_ALL_LOW);

    esp_deep_sleep_start();  // ZzzZZZzzzZZ
}

static void gps_event_handler(void *event_handler_arg,
                              esp_event_base_t event_base,
                              int32_t event_id,
                              void *event_data) {
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
        ESP_LOGI(T,
                 "%2d/%2d, %d/%d/%d %2d:%2d:%2d, %.05f°N, %.05f°E, %.02f m, +- %.02f m",
                 gps->sats_in_use,
                 gps->sats_in_view,
                 gps->date.year + 2000,
                 gps->date.month,
                 gps->date.day,
                 gps->tim.hour,
                 gps->tim.minute,
                 gps->tim.second,
                 gps->latitude,
                 gps->longitude,
                 gps->altitude,
                 gps->dop_p);
        break;
    case GPS_UNKNOWN:
        ESP_LOGW(T, "Unknown statement: %s", (char *)event_data);
        break;
    default:
        break;
    }
}

void gps_init() {
    nmea_parser_config_t config = {.uart = {.uart_port = UART_NUM_1,
                                            .rx_pin = P_GPS_RX,
                                            .tx_pin = P_GPS_TX,
                                            .baud_rate = 9600,
                                            .data_bits = UART_DATA_8_BITS,
                                            .parity = UART_PARITY_DISABLE,
                                            .stop_bits = UART_STOP_BITS_1,
                                            .event_queue_size = 16}};
    nmea_hdl = nmea_parser_init(&config);
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
    gps_wake(nmea_hdl);
}

void velogen_init() {
    gpio_set_direction(P_AUX_PWR, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(P_EN1, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(P_EN2, GPIO_MODE_INPUT);  // can be an input only :p
    gpio_set_direction(P_AC, GPIO_MODE_INPUT);
    gpio_set_direction(P_BOOT0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(P_BOOT0, GPIO_PULLUP_ONLY);

    setAuxPower(1);

    counter_init();

    cJSON *s = getSettings();

    // init shunt
    inaInit();
    inaBus32(false);
    inaPga(0);
    inaAvg(7);

    sleepTimeout = jGetI(s, "sleep_timeout", 30) * 1000 / portTICK_PERIOD_MS;

    gps_init();
    initWifi();
    startWebServer();
    mqtt_init();
    cache_init();  // open / create cache file on SPIFFS

    // init led strip last, so power can stabilize
    ws2812_init();
}

// main loop, called precisely every 50 ms
void velogen_loop() {
    static int frm = 0;

    int curTs = xTaskGetTickCount();
    static int ts_sleep = 0;
    static int ts_con =
        300000 / portTICK_PERIOD_MS;  // last TS when wheel moved / wanted to connect

    g_mVolts = inaV();
    g_mAmps = inaI();

    if (counter_read()) {
        // If wheel was moved
        ts_sleep = curTs;
        ts_con = curTs;
    }

    if ((frm % 100) == 0) {
        ESP_LOGD(T, "%d mV,  %d mA, %d cnt", g_mVolts, g_mAmps, g_wheelCnt);

        // we stopped, try to connect to wifi after 10s
        if (((curTs - ts_con) > (10000 / (int)portTICK_PERIOD_MS)) &&
            wifi_state == WIFI_NOT_CONNECTED) {
            tryJsonConnect();
            // don't try to re-connect in the next 5 minutes
            ts_con += sleepTimeout;
        }

        if (gpio_get_level(P_BOOT0) == 0) {
            gps_sleep(nmea_hdl);

            // if (wifi_state == WIFI_AP_MODE)
            //     tryJsonConnect();
            // else
            //     tryApMode();
        }

        if ((curTs - ts_sleep) > sleepTimeout)
            velogen_sleep(false);
    }

    // 20 Hz max.
    cache_handle();

    // TODO: better battery protection (shunt R maybe?)
    if (g_mVolts > 8400)
        ws2812_white();
    else
        ws2812_animate();

    frm++;
}
