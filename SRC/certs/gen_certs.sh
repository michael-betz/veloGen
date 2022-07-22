#!/bin/bash
set -e
# cp /etc/ssl/certs/USERTrust_RSA_Certification_Authority.pem ./root_cert.pem
openssl s_client -showcerts -connect falafel.uk.to:8883 </dev/null 2>/dev/null|openssl x509 -outform PEM > root_cert.pem
echo "For Common Name (CN), enter the hostname from where OTA updates will be served"
openssl req -x509 -newkey rsa:2048 -keyout ota_ca_key.pem -out ota_ca_cert.pem -days 365 -nodes
