#include <stdint.h>
#include <stdbool.h>
#include "Arduino.h"
#include "Wire.h"
#include "ssd1306.h"

#define I2C_ADDR 0x3C
#define FB_SIZE DISPLAY_WIDTH * DISPLAY_HEIGHT / 8

#define SET_CONTRAST 0x81
#define SET_ENTIRE_ON 0xA4
#define SET_NORM_INV 0xA6
#define SET_DISP 0xAE
#define SET_MEM_ADDR 0x20
#define SET_COL_ADDR 0x21
#define SET_PAGE_ADDR 0x22
#define SET_DISP_START_LINE 0x40
#define SET_SEG_REMAP 0xA0
#define SET_MUX_RATIO 0xA8
#define SET_COM_OUT_DIR 0xC0
#define SET_DISP_OFFSET 0xD3
#define SET_COM_PIN_CFG 0xDA
#define SET_DISP_CLK_DIV 0xD5
#define SET_PRECHARGE 0xD9
#define SET_VCOM_DESEL 0xDB
#define SET_CHARGE_PUMP 0x8D

static const uint8_t send_dat[] = {
	0x00,
	SET_COL_ADDR, 0, DISPLAY_WIDTH - 1,
	SET_PAGE_ADDR, 0, DISPLAY_HEIGHT / 8 - 1
};

// framebuffer with 8 pixels / byte
// the first byte sets up the oled for a frambuffer write
static uint8_t frameBuff[FB_SIZE + 1] = { 0x40 };

// actual framebuffer data starts here
uint8_t *g_frameBuff = &frameBuff[1];

static const uint8_t init_dat[] = {
	SET_DISP | 0x00,  // off
	// address setting
	SET_MEM_ADDR,
	0x00,  // horizontal
	// resolution and layout
	SET_DISP_START_LINE | 0x00,
	SET_SEG_REMAP | 0x01,  // column addr 127 mapped to SEG0
	SET_MUX_RATIO,
	DISPLAY_HEIGHT - 1,
	SET_COM_OUT_DIR | 0x08,  // scan from COM[N] to COM0
	SET_DISP_OFFSET,
	0x00,
	SET_COM_PIN_CFG,
	0x12,
	// timing and driving scheme
	SET_DISP_CLK_DIV,
	0x80,
	SET_PRECHARGE,
	0xF1,
	SET_VCOM_DESEL,
	0x30,  // 0.83*Vcc
	// display
	SET_CONTRAST,
	0xFF,  // maximum
	SET_ENTIRE_ON,  // output follows RAM contents
	SET_NORM_INV,  // not inverted
	// charge pump
	SET_CHARGE_PUMP,
	0x14,
	SET_DISP | 0x01,
};

static void cmd(uint8_t cmd)
{
	Wire.beginTransmission(I2C_ADDR);
	Wire.write(0x80);
	Wire.write(cmd);
	Wire.endTransmission();
}

void ssd_init()
{
	Wire.beginTransmission(I2C_ADDR);
	Wire.write(0x00);
	Wire.write(init_dat, sizeof(init_dat));
	Wire.endTransmission();
}

void ssd_poweroff()
{
	cmd(SET_DISP | 0x00);
}

void ssd_poweron()
{
	cmd(SET_DISP | 0x01);
}

void ssd_contrast(uint8_t val)
{
	cmd(SET_CONTRAST);
	cmd(val);
}

void ssd_invert(bool val)
{
	cmd(SET_NORM_INV | (val & 1));
}

void ssd_send()
{
	Wire.writeTransmission(I2C_ADDR, (uint8_t*)send_dat, sizeof(send_dat), true);
	Wire.writeTransmission(I2C_ADDR, frameBuff, sizeof(frameBuff), true);
}

// Set or clear a 1 bit pixel in framebuffer
void setPixel(unsigned x, unsigned y, bool isSet)
{
	// screen clipping
	if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
		return;

	unsigned byte = x + DISPLAY_WIDTH * (y >> 3);
	uint8_t mask = 1 << (y & 7);

	if (isSet)
		g_frameBuff[byte] |= mask;
	else
		g_frameBuff[byte] &= ~mask;
}

static void limit(int *a, int *b, int lim) {
	if (*a < 0)
		*a = 0;
	if (*a > lim)
		*a = lim;
	if (*b < 0)
		*b = 0;
	if (*b > lim)
		*b = lim;
	if (*a > *b) {
		int c = *a;
		*a = *b;
		*b = c;
	}
}

// get a 1 bit pixel-value from framebuffer
bool getPixel(unsigned x, unsigned y)
{
	if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
		return false;
	unsigned byte = x + DISPLAY_WIDTH * (y >> 3);
	uint8_t mask = 1 << (y & 7);
	return g_frameBuff[byte] & mask;
}

// set all pixels to on / off
void fill(bool val)
{
	uint8_t tmp = val ? 0xFF : 0x00;
	memset(g_frameBuff, tmp, FB_SIZE);
}

// no checks! Too bad.
static void hLine(unsigned x, unsigned y, unsigned w, bool isSet)
{
	uint8_t *pBuf = &g_frameBuff[(y / 8) * DISPLAY_WIDTH + x], mask = 1 << (y & 7);
	if (isSet){
		while (w--)
			*pBuf++ |= mask;
	} else {
		mask = ~mask;
		while (w--)
			*pBuf++ &= mask;
	}
}

// no checks!
static void vLine(unsigned x, unsigned y, unsigned h, bool isSet)
{
	if (h <= 0) return;
	uint8_t *pBuf = &g_frameBuff[(y / 8) * DISPLAY_WIDTH + x];

	// do the first partial byte, if necessary - this requires some masking
	uint8_t mod = (y & 7);
	if (mod) {
		// mask off the high n bits we want to set
		mod = 8 - mod;
		// uint8_t mask = ~(0xFF >> mod);
		static const uint8_t premask[8] = {
			0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE
		};
		uint8_t mask = premask[mod];
		// adjust the mask if we're not going to reach the end of this byte
		if (h < mod)
			mask &= (0XFF >> (mod - h));
		if (isSet)
			*pBuf |= mask;
		else
			*pBuf &= ~mask;
		pBuf += DISPLAY_WIDTH;
	}

	if (h >= mod) { // More to go?
		h -= mod;
		// Write solid bytes while we can - effectively 8 rows at a time
		if (h >= 8) {
			uint8_t val = isSet ? 0xFF : 0;
			do {
				*pBuf = val;   // Set byte
				pBuf += DISPLAY_WIDTH; // Advance pointer 8 rows
				h -= 8;        // Subtract 8 rows from height
			} while (h >= 8);
		}
	}

	if (h) { // Do the final partial byte, if necessary
		mod = h & 7;
		// this time we want to mask the low bits of the byte,
		// vs the high bits we did above
		// uint8_t mask = (1 << mod) - 1;
		static const uint8_t postmask[8] = {
			0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
		};
		uint8_t mask = postmask[mod];
		if (isSet) {
			*pBuf |= mask;
		} else {
			*pBuf &= ~mask;
		}
	}
}

void fillRect(int x0, int x1, int y0, int y1, bool isSet)
{
	limit(&x0, &x1, DISPLAY_WIDTH);
	limit(&y0, &y1, DISPLAY_HEIGHT);
	unsigned h = y1 - y0 + 1;
	for (unsigned x=x0; x<=x1; x++)
		vLine(x, y0, h, isSet);
}

void rect(int x0, int x1, int y0, int y1, bool isSet)
{
	limit(&x0, &x1, DISPLAY_WIDTH);
	limit(&y0, &y1, DISPLAY_HEIGHT);
	unsigned w = x1 - x0;
	unsigned h = y1 - y0;
	hLine(x0, y0, w, isSet);
	hLine(x0, y0 + h, w, isSet);
	vLine(x0, y0, h + 1, isSet);
	vLine(x0 + w, y0, h + 1, isSet);
}
