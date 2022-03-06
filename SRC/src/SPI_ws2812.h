/*
 * SPI_ws2812.h
 *
 *  Created on: 02-Nov-2020
 *      Author: Dhananjay Khairnar
 */

#ifndef MAIN_SPI_WS2812_H_
#define MAIN_SPI_WS2812_H_
#include <stdio.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

void initSPIws2812();
void led_strip_update(uint32_t *buf);


#endif /* MAIN_SPI_WS2812_H_ */
