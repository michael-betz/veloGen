#!/bin/bash
# Start https server for updating firmware over wifi
# add the hostname of your PC to settings.json
# trigger the update on the OTA screen (5) on velogen
set -e
pio run
(cd .pio/build/velogen/; openssl s_server -WWW -key ../../../certs/ota_void_ca_key.pem -cert ../../../certs/ota_void_ca_cert.pem -port 8000)
