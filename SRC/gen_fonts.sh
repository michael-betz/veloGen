#!/bin/bash
set -e
lv_font_conv --size 47 --bpp 1 --format lvgl --font ../OLD/ARDUINO/fonts/ConcertOne-Regular.ttf -r 0x20-0x7F --no-kerning -o src/concert_one.c
lv_font_conv --size 35 --bpp 1 --format lvgl --font ../OLD/ARDUINO/fonts/FontAwesome5-Solid+Brands+Regular.woff -r 0xf3fd,0xf0e7,0xf1e6,0xf5df --no-kerning -o src/fa.c

for fName in concert_one fa; do
	sed -i 's/#include "lvgl\/lvgl.h"/#include "lv_font.h"/' src/$fName.c
done
