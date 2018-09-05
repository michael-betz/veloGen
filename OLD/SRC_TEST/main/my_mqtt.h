#ifndef MY_MQTT_H
#define MY_MQTT_H

#define CONNECTED_BIT BIT0
extern EventGroupHandle_t wifi_event_group;
extern mqtt_client *client;

extern void wifi_conn_init(void);

#endif
