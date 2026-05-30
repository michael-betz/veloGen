#ifndef STATIC_WS_H
#define STATIC_WS_H

#include "esp_err.h"
#include "esp_http_server.h"

void startWebServer();
void stopWebServer();

// override this in main!
esp_err_t ws_callback(httpd_req_t *req, httpd_ws_frame_t *frame);

#endif
