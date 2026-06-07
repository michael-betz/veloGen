#include "gps.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "json_settings.h"
#include "lwip/sockets.h"
#include "main.h"
#include "velo.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

static const char *T = "GPS";

#define UART_NUM UART_NUM_1

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
                 "%2d/%2d, %d/%d/%d %02d:%02d:%02d, %.05f°N, %.05f°E, %.02f m, +- %.02f",
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

        char tmp[64];
        int strlen = 0;

        for (int i = 0; i < MIN(gps->sats_in_view, GPS_MAX_SATELLITES_IN_VIEW); i++) {
            strlen += snprintf(
                &tmp[strlen], sizeof(tmp) - strlen, "%02d ", gps->sats_desc_in_view[i].snr);
            if (strlen >= sizeof(tmp) - 1)
                break;
        }
        ESP_LOGD(T, "SNR %s", tmp);
        break;
    case GPS_UNKNOWN:
        ESP_LOGW(T, "%s", (char *)event_data);
        break;
    default:
        break;
    }
}

/**
 * @brief Appends the 2-byte Fletcher Checksum and sends the packet over UART
 */
static void send_ubx_packet(const uint8_t *msg, uint16_t length) {
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;

    // Checksum skips first 2 sync bytes (0xB5, 0x62)
    for (uint16_t i = 2; i < length; i++) {
        ck_a += msg[i];
        ck_b += ck_a;
    }
    uint8_t tmp[2] = {ck_a, ck_b};

    uart_write_bytes(UART_NUM, msg, length);
    uart_write_bytes(UART_NUM, tmp, 2);
}

void gps_init_parser() {
    if (nmea_hdl != NULL)
        return;

    gpio_deep_sleep_hold_dis();
    gpio_hold_dis(P_GPS_RX);
    gpio_reset_pin(P_GPS_RX);

    nmea_parser_config_t config = {.uart = {.uart_port = UART_NUM,
                                            .rx_pin = P_GPS_RX,
                                            .tx_pin = P_GPS_TX,
                                            .baud_rate = jGetI(getSettings(), "gps_baudrate", 9600),
                                            .data_bits = UART_DATA_8_BITS,
                                            .parity = UART_PARITY_DISABLE,
                                            .stop_bits = UART_STOP_BITS_1,
                                            .event_queue_size = 16}};
    nmea_hdl = nmea_parser_init(&config);
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
    gps_wake();
}

void gps_deinit_parser() {
    if (nmea_hdl != NULL) {
        nmea_parser_deinit(nmea_hdl);
        nmea_hdl = NULL;
    } else {
        uart_driver_delete(UART_NUM);
    }
}

void gps_sleep() {
    ESP_LOGI(T, "Powering down GPS module");
    gpio_set_direction(P_GPS_EN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(P_GPS_EN, 1);
    gpio_hold_en(P_GPS_EN);
    gpio_deep_sleep_hold_en();
}

void gps_wake() {
    ESP_LOGI(T, "Powering up GPS module");
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis(P_GPS_EN);
    gpio_set_direction(P_GPS_EN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(P_GPS_EN, 0);
}

// --------------------------------
//  TCP Proxy for ustudio
// --------------------------------

#define BUF_SIZE 512
#define TCP_PORT 2947
#define POLL_TIMEOUT 30  // [ms]

static TaskHandle_t proxy_task_handle = NULL;
static SemaphoreHandle_t proxy_done_sem = NULL;
static volatile bool is_proxy_running = false;
static int listen_sock = -1;

static void gps_proxy_task(void *arg) {
    static uint8_t buf[BUF_SIZE];

    /* --- UART init --- */
    uart_config_t uart_cfg = {
        .baud_rate = jGetI(getSettings(), "gps_baudrate", 9600),
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_cfg));
    ESP_ERROR_CHECK(
        uart_set_pin(UART_NUM, P_GPS_TX, P_GPS_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    /* --- TCP listen socket --- */
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(TCP_PORT),
    };
    bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_sock, 1);

    ESP_LOGI(T, "Listening on port %d", TCP_PORT);

    while (is_proxy_running) {
        // Block here until u-center connects
        struct sockaddr_in client;
        socklen_t clen = sizeof(client);
        int sock = accept(listen_sock, (struct sockaddr *)&client, &clen);
        if (sock < 0) {
            // Either stop() closed the listen socket, or a real error
            if (!is_proxy_running)
                break;  // clean shutdown
            ESP_LOGE(T, "accept() failed: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        ESP_LOGI(T, "Client connected: %s", inet_ntoa(client.sin_addr));

        // Short recv timeout so we can interleave UART polling
        struct timeval tv = {.tv_usec = POLL_TIMEOUT * 1000};  // 30 ms
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        uart_flush(UART_NUM);

        // --- Proxy loop ---
        while (is_proxy_running) {
            // TCP → UART
            int n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                uart_write_bytes(UART_NUM, buf, n);
                ESP_LOG_BUFFER_HEXDUMP(T, buf, n, ESP_LOG_DEBUG);
            } else if (n == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
                break;
            }

            // UART → TCP
            n = uart_read_bytes(UART_NUM, buf, sizeof(buf), pdMS_TO_TICKS(POLL_TIMEOUT));
            if (n > 0 && send(sock, buf, n, 0) < 0)
                break;
        }

        ESP_LOGI(T, "Client disconnected");
        close(sock);
    }
    uart_driver_delete(UART_NUM);
    ESP_LOGI(T, "Proxy task exiting");
    xSemaphoreGive(proxy_done_sem);
    vTaskDelete(NULL);
}

void gps_proxy_start(void) {
    if (is_proxy_running)
        return;
    if (proxy_done_sem == NULL)
        proxy_done_sem = xSemaphoreCreateBinary();
    is_proxy_running = true;
    xTaskCreate(gps_proxy_task, "gps_proxy", 4096, NULL, 5, &proxy_task_handle);
}

void gps_proxy_stop(void) {
    if (!is_proxy_running)
        return;

    is_proxy_running = false;

    // Unblocks accept() and any blocking recv() immediately
    if (listen_sock >= 0) {
        close(listen_sock);
        listen_sock = -1;
    }

    // Wait for task to confirm it has exited
    xSemaphoreTake(proxy_done_sem, pdMS_TO_TICKS(3000));
    vSemaphoreDelete(proxy_done_sem);
    proxy_done_sem = NULL;
    proxy_task_handle = NULL;
}

void gps_init_from_json() {
    cJSON *s = getSettings();
    if (jGetB(s, "gps_debug_enabled", false)) {
        ESP_LOGW(T, "!!! GPS DEBUG MODE ENABLED !!!");
        ESP_LOGW(T, "Connect u-center to tcp://%s:2947", jGetS(s, "hostname", WIFI_HOST_NAME));
        ESP_LOGW(T, "Sleep timeout set to 1 h");
        gps_deinit_parser();
        gps_proxy_start();
        gps_wake();
        g_sleepTimeout = 3600 * 1000 / portTICK_PERIOD_MS;
    } else {
        gps_proxy_stop();
        gps_init_parser();
    }
}
