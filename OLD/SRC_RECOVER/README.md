# `Web-Bootloader` firmware

This app is flashed to the first OTA partition, starts a webserver and receives a .bin file via

```bash
curl --data-binary @$(APP_BIN) http://velogen/flash/upload;
curl http://velogen/flash/reboot
```

It always writes to the ota1 partition (the second one) and reboots into this newly written image.
