#!/bin/bash
# Start https server for updating firmware over wifi
# add the hostname of your PC to settings.json
# trigger the update on the OTA screen (5) on velogen
set -e
pio run
# (cd .pio/build/velogen/; openssl s_server -WWW -key ../../../certs/ota_ca_key.pem -cert ../../../certs/ota_ca_cert.pem -port 8000)
python3 -m http.server --directory .pio/build/velogen/ 8000
