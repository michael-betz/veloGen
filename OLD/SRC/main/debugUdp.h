/*
 * debugUdp.h
 *
 *  Created on: Apr 5, 2017
 *      Author: michael
 */

#ifndef MAIN_DEBUGUDP_H_
#define MAIN_DEBUGUDP_H_

#define UDP_DEBUG_SERVER "kimchi"
#define UDP_DEBUG_PORT    4711
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
#define CONNECTED_BIT BIT0

void wifiInit();
void udp_debug_init(void *pvParameters);


#endif /* MAIN_DEBUGUDP_H_ */
