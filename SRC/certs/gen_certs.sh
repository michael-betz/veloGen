#!/bin/bash
set -e
cp /etc/ssl/certs/DST_Root_CA_X3.pem .
echo "For Common Name (CN), enter the hostname from where OTA updates will be served"
openssl req -x509 -newkey rsa:2048 -keyout ota_ca_key.pem -out ota_ca_cert.pem -days 365 -nodes
