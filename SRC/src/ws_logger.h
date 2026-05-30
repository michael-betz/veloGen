#pragma once

#include "esp_http_server.h"

// -------------------
//  websocket logging
// -------------------
void init_ws_logger();

// dump the whole RTC buffer to the WS, oldest entries first.
// call this once the WS connection is open.
void wsDumpRtc(httpd_req_t *req, bool dump_all);
