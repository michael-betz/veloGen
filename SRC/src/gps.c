#include "gps.h"
#include "esp_log.h"
#include "main.h"
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

static const char *T = "GPS";

gps_t g_gps_data = {0};

static nmea_parser_handle_t nmea_hdl = NULL;

static void sync_system_time_from_gps(const gps_t *gps) {
    if (!gps->valid) {
        ESP_LOGW(T, "GPS data is not valid, skipping time sync.");
        return;
    }

    struct tm timeinfo = {0};

    // NMEA parser starts from 2000
    // struct tm expects years since 1900.
    timeinfo.tm_year = gps->date.year + 100;

    // NMEA parser starts from 1. struct tm expects 0-11.
    timeinfo.tm_mon = gps->date.month - 1;
    timeinfo.tm_mday = gps->date.day;

    timeinfo.tm_hour = gps->tim.hour;
    timeinfo.tm_min = gps->tim.minute;
    timeinfo.tm_sec = gps->tim.second;

    // Temporarily set timezone to UTC to ensure mktime calculates the epoch correctly
    char *old_tz = getenv("TZ");
    setenv("TZ", "UTC0", 1);
    tzset();

    time_t epoch_seconds = mktime(&timeinfo);

    if (epoch_seconds != -1) {
        struct timeval tv;
        tv.tv_sec = epoch_seconds;
        tv.tv_usec = gps->tim.thousand * 1000;

        if (settimeofday(&tv, NULL) == 0)
            ESP_LOGD(T, "epoch_seconds: %lld", (long long)epoch_seconds);
        else
            ESP_LOGE(T, "Failed to set system time.");

    } else {
        ESP_LOGE(T, "Failed to convert GPS time to epoch.");
    }

    // Restore the previous timezone setting
    if (old_tz)
        setenv("TZ", old_tz, 1);
    else
        unsetenv("TZ");

    tzset();
}

static void gps_event_handler(void *event_handler_arg,
                              esp_event_base_t event_base,
                              int32_t event_id,
                              void *event_data) {
    static int i = 0;
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        memcpy(&g_gps_data, gps, sizeof(gps_t));

        if (gps->valid) {
            if ((i++ % 128) == 0)
                sync_system_time_from_gps(gps);
        } else {
            i = 0;
        }

        /* print information parsed from GPS statements */
        ESP_LOGD(T,
                 "%2d/%2d, %d/%d/%d %02d:%02d:%02d, %.05f°N, %.05f°E, %.02f m, +- %.02fh %.02fp",
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
                 gps->dop_h,
                 gps->dop_p);
        break;
    case GPS_UNKNOWN:
        ESP_LOGW(T, "%s", (char *)event_data);
        break;
    default:
        break;
    }
}

void gps_init() {
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis(P_GPS_RX);
    gpio_reset_pin(P_GPS_RX);

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
    gps_wake();
}

void gps_sleep() {
    // const uint8_t sleep_cmd[] = {0xB5,
    //                              0x62,
    //                              0x02,
    //                              0x41,
    //                              0x08,
    //                              0x00,
    //                              0x00,
    //                              0x00,
    //                              0x00,
    //                              0x00,
    //                              0x02,
    //                              0x00,
    //                              0x00,
    //                              0x00,
    //                              0x4D,
    //                              0x3B};

    // uart_write_bytes(UART_NUM_1, (const char *)sleep_cmd, sizeof(sleep_cmd));
    // ESP_LOGI(T, "GPS sleep command sent");

    // // Keep GPS UART RX pin high during sleep to prevent it from waking up
    // gpio_reset_pin(P_GPS_RX);
    // gpio_set_direction(P_GPS_RX, GPIO_MODE_OUTPUT);
    // gpio_set_level(P_GPS_RX, 1);
    // gpio_hold_en(P_GPS_RX);
    // gpio_deep_sleep_hold_en();

    gpio_set_direction(P_GPS_EN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(P_GPS_EN, 1);
    gpio_hold_en(P_GPS_EN);
    gpio_deep_sleep_hold_en();
}

void gps_wake() {
    // const char dummy_byte = 0xFF;

    // uart_write_bytes(UART_NUM_1, &dummy_byte, 1);
    // ESP_LOGI(T, "GPS wake byte sent");

    gpio_deep_sleep_hold_dis();
    gpio_hold_dis(P_GPS_EN);
    gpio_set_direction(P_GPS_EN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(P_GPS_EN, 0);
}
