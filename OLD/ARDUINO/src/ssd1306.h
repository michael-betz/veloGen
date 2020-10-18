#ifndef SSD1306_H
#define SSD1306_H

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT  64
#define LV_BPP 1  // bits / pixel

void ssd_init();
void ssd_poweroff();
void ssd_poweron();
void ssd_contrast(uint8_t val);
void ssd_invert(bool val);
void ssd_send();

// SET / GET a single pixel in the framebuffer
void setPixel(unsigned x, unsigned y, bool isSet);
bool getPixel(unsigned x, unsigned y);

// Set whole screen to fixed value
void fill(bool val);
void fillRect(int x0, int x1, int y0, int y1, bool isSet);
void rect(int x0, int x1, int y0, int y1, bool isSet);

#endif
