#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "gps.h"
#include "json_settings.h"
#include "main.h"
#include "mqtt_client.h"
#include "velo.h"
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

static const char *T = "MQTT_CACHE";

// format of one measurement
typedef struct {
    uint32_t ts;
    uint16_t volts;
    int16_t amps;
    uint16_t speed;
    uint32_t cnt;
    float latitude;
    float longitude;
    float altitude;
    float pos_dilution;
} t_datum;
#define BLOCK_SIZE sizeof(t_datum)

#define FILE_PATH (F_PREFIX "/telemetry.bin")
#define FILE_SEND_PATH (F_PREFIX "/sending.bin")

// Number of records to send in a single MQTT payload
#define CHUNK_SIZE 32

// Global state variables
FILE *record_file = NULL;
SemaphoreHandle_t telemetry_mutex = NULL;
static esp_mqtt_client_handle_t mqtt_c = NULL;
static SemaphoreHandle_t ack_sem = NULL;

static atomic_bool mqtt_connected = ATOMIC_VAR_INIT(false);
static atomic_int pending_ack_id = ATOMIC_VAR_INIT(-1);

// initialized from .json
static bool is_cache_enabled = false;
static const char *mqtt_topic = NULL;
static int meas_ticks = 1;

static void transmit_backlog_task(void *pvParameters) {
    while (1) {
        // Rotate currently acquiring file to sending file ...
        if (xSemaphoreTake(telemetry_mutex, portMAX_DELAY) == pdTRUE) {
            struct stat st;

            // stat returns -1 if the file doesn't EXISTS.
            // Then we can close and rename FILE_PATH to FILE_SEND_PATH.
            if (stat(FILE_SEND_PATH, &st) != 0) {
                if (record_file != NULL) {
                    fclose(record_file);
                    record_file = NULL;
                }
                // This will fail harmlessly if FILE_PATH doesn't exist either
                rename(FILE_PATH, FILE_SEND_PATH);
            }
            xSemaphoreGive(telemetry_mutex);
        }

        // Attempt to open the sending file
        FILE *send_file = fopen(FILE_SEND_PATH, "rb");
        if (send_file == NULL) {
            ESP_LOGI(T, "No more backlog to send. Task finished.");
            break;  // Exit the while loop and end the task
        }

        t_datum buffer[CHUNK_SIZE];
        size_t read_count;
        bool transmission_success = true;

        ESP_LOGI(T, "Transmitting backlog chunk...");

        // 3. Transmit the file
        while ((read_count = fread(buffer, BLOCK_SIZE, CHUNK_SIZE, send_file)) > 0) {
            if (!atomic_load(&mqtt_connected)) {
                ESP_LOGW(T, "Lost connection before sending. Halting.");
                transmission_success = false;
                break;
            }

            // Clear the semaphore in case of lingering triggers
            xSemaphoreTake(ack_sem, 0);

            int msg_id = esp_mqtt_client_publish(
                mqtt_c, mqtt_topic, (const char *)buffer, read_count * BLOCK_SIZE, 1, 0);

            if (msg_id >= 0) {
                atomic_store(&pending_ack_id, msg_id);
                if (xSemaphoreTake(ack_sem, pdMS_TO_TICKS(30000)) != pdTRUE) {
                    ESP_LOGE(T, "Timeout waiting for MQTT ACK. Halting.");
                    transmission_success = false;
                    break;
                }
            } else {
                ESP_LOGE(T, "Failed to enqueue MQTT message. Halting.");
                transmission_success = false;
                break;
            }
        }

        fclose(send_file);
        atomic_store(&pending_ack_id, -1);

        // Cleanup and loop
        if (transmission_success) {
            unlink(FILE_SEND_PATH);
            ESP_LOGI(T, "Backlog chunk complete and deleted.");
            // The loop will now repeat. If FILE_PATH accumulated new data
            // during this upload, it will be rotated and sent next!
        } else {
            // Network failed. Leave FILE_SEND_PATH intact so we can resume later.
            break;
        }
    }

    vTaskDelete(NULL);
}

static void
cb_mqtt_pub(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    if (atomic_load(&pending_ack_id) == event->msg_id)
        xSemaphoreGive(ack_sem);
}

static void
cb_mqtt_con(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGI(T, "MQTT connected");
    atomic_store(&mqtt_connected, true);
    xTaskCreate(transmit_backlog_task, "mqtt_transmit", 4096, NULL, 5, NULL);
}

static void
cb_mqtt_discon(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGI(T, "MQTT disconnected");
    atomic_store(&mqtt_connected, false);
}

static void got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    esp_mqtt_client_reconnect(mqtt_c);
}

void cache_init() {
    telemetry_mutex = xSemaphoreCreateMutex();
    ack_sem = xSemaphoreCreateBinary();
    cJSON *s = getSettings();
    meas_ticks = jGetI(s, "meas_ticks", 20);  // 0 = off, otherwise [.05 s]
    mqtt_topic = jGetS(s, "mqtt_topic", "velogen/raw");
    is_cache_enabled = jGetB(s, "mqtt_cache_enabled", false);

    // MQTT client
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = jGetS(s, "mqtt_url", "null");
    mqtt_cfg.credentials.client_id = jGetS(s, "hostname", WIFI_HOST_NAME);
    mqtt_cfg.broker.verification.crt_bundle_attach = esp_crt_bundle_attach;
    mqtt_cfg.task.priority = 1;
    // mqtt_cfg.network.reconnect_timeout_ms = 300;
    mqtt_cfg.network.disable_auto_reconnect = true;
    ESP_LOGI(T, "Publishing to %s, %s", mqtt_cfg.broker.address.uri, mqtt_topic);

    mqtt_c = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqtt_c) {
        ESP_LOGE(T, "Error initializing mqtt client");
        return;
    }

    // register for MQTT events
    E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_CONNECTED, cb_mqtt_con, NULL));
    E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_DISCONNECTED, cb_mqtt_discon, NULL));
    E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_PUBLISHED, cb_mqtt_pub, mqtt_c));
    E(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip, NULL));

    esp_mqtt_client_start(mqtt_c);
}

static void save_telemetry_offline(const t_datum *datum) {
    if (record_file == NULL) {
        record_file = fopen(FILE_PATH, "ab");
        if (!record_file) {
            ESP_LOGE(T, "Failed to open file for appending");
            return;
        }
    }
    fwrite(datum, BLOCK_SIZE, 1, record_file);
}

void handle_new_measurement(const t_datum *datum) {
    // Lock-free check if we are online
    if (atomic_load(&mqtt_connected)) {
        esp_mqtt_client_publish(mqtt_c, mqtt_topic, (const char *)datum, BLOCK_SIZE, 1, 0);
    } else if (is_cache_enabled) {
        // We are offline, take the mutex to protect the file sequence
        if (xSemaphoreTake(telemetry_mutex, portMAX_DELAY) == pdTRUE) {
            save_telemetry_offline(datum);
            xSemaphoreGive(telemetry_mutex);
        }
    }
}

void cache_handle() {
    static unsigned seq = 0;

    // shall we take a new data point?
    if (!(meas_ticks > 0 && (seq++ % meas_ticks) == 0))
        return;

    ESP_LOGD(T, "%d mV,  %d mA, %d cnt", g_mVolts, g_mAmps, g_wheelCnt);

    // collect a new data point
    t_datum datum;
    datum.ts = time(NULL);  // TODO need higher resolution timestamps
    datum.volts = g_mVolts;
    datum.amps = g_mAmps;
    datum.speed = (uint16_t)g_speed;  // [km/h * 10]
    datum.cnt = g_wheelCnt;
    // GPS data
    datum.longitude = g_gps_data.longitude;
    datum.latitude = g_gps_data.latitude;
    datum.altitude = g_gps_data.altitude;
    datum.pos_dilution = g_gps_data.dop_p;

    handle_new_measurement(&datum);
}
