#pragma once

void ws2812_init(void);
void ws2812_off();
void ws2812_animate();
void ws2812_white();

// Blink a color for a certain time as a status indicator
void ws2812_indicate(int ticks, unsigned color);
