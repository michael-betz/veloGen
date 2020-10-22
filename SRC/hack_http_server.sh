#!/bin/bash
# Update esp_http_server component to latest version to get websocket support
set -e
IDF_PATH=$HOME/.platformio/packages/framework-espidf/
git clone https://github.com/espressif/esp-idf.git
mv $IDF_PATH/components/esp_http_server $IDF_PATH/esp_http_server_bak
mv esp-idf/components/esp_http_server $IDF_PATH/components/
patch $IDF_PATH/components/esp_http_server/CMakeLists.txt remove_timer.patch
rm -rf esp-idf/
