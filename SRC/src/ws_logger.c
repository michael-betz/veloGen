#include "ws_logger.h"
#include "esp_log.h"
#include "rom/rtc.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// -----------------------------------------------
//  Websocket logger
// -----------------------------------------------
// Rolling buffer size in RTC mem for log entries in bytes
#define LOG_FILE_SIZE 3576

static const char *T = "WS_LOG";

// put the buffer and write pointer in RTC memory,
// such that it survives sleep mode
RTC_NOINIT_ATTR static char rtcLogBuffer[LOG_FILE_SIZE];
RTC_NOINIT_ATTR static char *rtcLogWritePtr = rtcLogBuffer;
RTC_NOINIT_ATTR static char *rtcLogReadPtr = rtcLogBuffer;

static const char *logBuffEnd = rtcLogBuffer + LOG_FILE_SIZE - 1;

// The original uart logger
vprintf_like_t log_original = NULL;

// add stdout chars to the RTC log buffer.
static int print_to_ws(const char *format, va_list arg) {
    static char charBuffer[255];

    if (log_original)
        log_original(format, arg);

    int charLen = vsnprintf(charBuffer, sizeof(charBuffer), format, arg);

    // ring-buffer in RTC memory for persistence in sleep mode
    for (unsigned i = 0; i < charLen; i++) {
        char c = charBuffer[i];

        *rtcLogWritePtr = c;
        if (rtcLogWritePtr >= logBuffEnd) {
            rtcLogWritePtr = rtcLogBuffer;
        } else {
            rtcLogWritePtr++;
        }
    }

    return charLen;
}

void init_ws_logger() {
    int reason = rtc_get_reset_reason(0);
    if (reason == POWERON_RESET) {
        ESP_LOGW(T, "Clearing RTC log");
        memset(rtcLogBuffer, 0, LOG_FILE_SIZE);
        rtcLogWritePtr = rtcLogBuffer;
        rtcLogReadPtr = rtcLogBuffer;
    } else {
        if (rtcLogWritePtr < rtcLogBuffer || rtcLogWritePtr > logBuffEnd)
            rtcLogWritePtr = rtcLogBuffer;

        if (rtcLogReadPtr < rtcLogBuffer || rtcLogReadPtr > logBuffEnd)
            rtcLogReadPtr = rtcLogBuffer;
    }

    // Keep a copy for printing to UART
    if (!log_original) {
        log_original = esp_log_set_vprintf(print_to_ws);
        ESP_LOGI(T, "Enabled web-socket logging 👋");
    }
}

void wsDumpRtc(httpd_req_t *req, bool dump_all) {
    int n_bytes = 0;

    if (dump_all) {
        n_bytes = LOG_FILE_SIZE;
        rtcLogReadPtr = rtcLogWritePtr;
    } else {
        n_bytes = (rtcLogWritePtr - rtcLogReadPtr + LOG_FILE_SIZE);
        n_bytes %= LOG_FILE_SIZE;
    }

    if (n_bytes < 0)
        n_bytes = 0;

    char *buffer = malloc(n_bytes + 1);
    if (!buffer)
        return;

    char *p = buffer;
    *p++ = 'a';

    for (unsigned i = 0; i < n_bytes; i++) {
        *p++ = *rtcLogReadPtr++;
        if (rtcLogReadPtr > logBuffEnd)
            rtcLogReadPtr = rtcLogBuffer;
    }

    httpd_ws_frame_t wsf = {0};
    wsf.type = HTTPD_WS_TYPE_TEXT;
    wsf.payload = (uint8_t *)buffer;
    wsf.len = n_bytes + 1;
    httpd_ws_send_frame(req, &wsf);

    free(buffer);
}
