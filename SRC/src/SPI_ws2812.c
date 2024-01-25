/*
 * SPI_ws2812.c
 *
 *  Created on: 02-Nov-2020
 *      Author: Dhananjay Khairnar
 */

#include "SPI_ws2812.h"
#include "velo.h"
#include "perlin.h"

// static const char *T = "LED_STRIP";

// how likely an LED is lit [0.0 .. 1.0]
float g_intensity = 0.1;

// LED strip colors
static uint32_t pixels[N_LEDS];

#define LED_DMA_BUFFER_SIZE ((N_LEDS * 16 * (24 / 4))) + 1

typedef struct {
	spi_host_device_t host;
	spi_device_handle_t spi;
	int dma_chan;
	spi_device_interface_config_t devcfg;
	spi_bus_config_t buscfg;
} SPI_settings_t;

uint16_t* ledDMAbuffer = NULL;

static SPI_settings_t SPI_settings = {
		.host = HSPI_HOST,
		.dma_chan = 2,
		.buscfg = {
				.miso_io_num = -1,
				.mosi_io_num = P_EN1,  // P_EN0,
				.sclk_io_num = -1,
				.quadwp_io_num = -1,
				.quadhd_io_num = -1,
				.max_transfer_sz = LED_DMA_BUFFER_SIZE
		},
		.devcfg = {
				.clock_speed_hz = 3.2 * 1000 * 1000,  // Clock out at 3.2 MHz
				.mode = 0,  // SPI mode 0
				.spics_io_num = -1,  // CS pin
				.queue_size = 1,  // Not sure if needed
				.command_bits = 0,
				.address_bits = 0
		}
};

// Perlin noise seeds
static int seed1, seed2, seed3;

void initSPIws2812()
{
	esp_err_t err;

	err = spi_bus_initialize(SPI_settings.host, &SPI_settings.buscfg, SPI_settings.dma_chan);
	ESP_ERROR_CHECK(err);

	// Attach the Accel to the SPI bus
	err = spi_bus_add_device(SPI_settings.host, &SPI_settings.devcfg, &SPI_settings.spi);
	ESP_ERROR_CHECK(err);

	if (ledDMAbuffer) {
		heap_caps_free(ledDMAbuffer);
		ledDMAbuffer = NULL;
	}
	ledDMAbuffer = heap_caps_malloc(LED_DMA_BUFFER_SIZE, MALLOC_CAP_DMA);  // Critical to be DMA memory.

	seed1 = rand();
	seed2 = rand();
	seed3 = rand();

	led_strip_off();
}

void led_strip_update(uint32_t *buf)
{
	uint16_t LedBitPattern[16] = {
	0x8888,
	0x8C88,
	0xC888,
	0xCC88,
	0x888C,
	0x8C8C,
	0xC88C,
	0xCC8C,
	0x88C8,
	0x8CC8,
	0xC8C8,
	0xCCC8,
	0x88CC,
	0x8CCC,
	0xC8CC,
	0xCCCC
	};
	uint32_t i;
	esp_err_t err;

	memset(ledDMAbuffer, 0, LED_DMA_BUFFER_SIZE);
	int n = 0;
	for (i = 0; i < N_LEDS; i++) {
		// Data you want to write to each LEDs, I'm my case it's 95 RGB x 3 color
		uint32_t temp = buf[i];

		// R
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp >>12)];
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp)>>8];

		// G
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp >>4)];
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp)];

		// B
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp >>20)];
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp)>>16];
	}

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = LED_DMA_BUFFER_SIZE * 8;  // length is in bits
	t.tx_buffer = ledDMAbuffer;

	err = spi_device_transmit(SPI_settings.spi, &t);
	ESP_ERROR_CHECK(err);
}

void led_strip_off()
{
	memset(pixels, 0, 4 * N_LEDS);
	led_strip_update(pixels);
}

// clip b such that: a <= b <= c
static float limit(float a, float b, float c)
{
	return ((a > b) ? a : (b > c) ? c : b);
}

static uint32_t colorHSV(uint16_t hue, uint8_t sat, uint8_t val)
{
  uint8_t r, g, b;

  // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
  // 0 is not the start of pure red, but the midpoint...a few values above
  // zero and a few below 65536 all yield pure red (similarly, 32768 is the
  // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
  // each for red, green, blue) really only allows for 1530 distinct hues
  // (not 1536, more on that below), but the full unsigned 16-bit type was
  // chosen for hue so that one's code can easily handle a contiguous color
  // wheel by allowing hue to roll over in either direction.
  hue = (hue * 1530L + 32768) / 65536;
  // Because red is centered on the rollover point (the +32768 above,
  // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
  // where 0 and 1530 would yield the same thing. Rather than apply a
  // costly modulo operator, 1530 is handled as a special case below.

  // So you'd think that the color "hexcone" (the thing that ramps from
  // pure red, to pure yellow, to pure green and so forth back to red,
  // yielding six slices), and with each color component having 256
  // possible values (0-255), might have 1536 possible items (6*256),
  // but in reality there's 1530. This is because the last element in
  // each 256-element slice is equal to the first element of the next
  // slice, and keeping those in there this would create small
  // discontinuities in the color wheel. So the last element of each
  // slice is dropped...we regard only elements 0-254, with item 255
  // being picked up as element 0 of the next slice. Like this:
  // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
  // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
  // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
  // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
  // the constants below are not the multiples of 256 you might expect.

  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if (hue < 510) { // Red to Green-1
    b = 0;
    if (hue < 255) { //   Red to Yellow-1
      r = 255;
      g = hue;       //     g = 0 to 254
    } else {         //   Yellow to Green-1
      r = 510 - hue; //     r = 255 to 1
      g = 255;
    }
  } else if (hue < 1020) { // Green to Blue-1
    r = 0;
    if (hue < 765) { //   Green to Cyan-1
      g = 255;
      b = hue - 510;  //     b = 0 to 254
    } else {          //   Cyan to Blue-1
      g = 1020 - hue; //     g = 255 to 1
      b = 255;
    }
  } else if (hue < 1530) { // Blue to Red-1
    g = 0;
    if (hue < 1275) { //   Blue to Magenta-1
      r = hue - 1020; //     r = 0 to 254
      b = 255;
    } else { //   Magenta to Red-1
      r = 255;
      b = 1530 - hue; //     b = 255 to 1
    }
  } else { // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 = 1 + val;  // 1 to 256; allows >>8 instead of /255
  uint16_t s1 = 1 + sat;  // 1 to 256; same reason
  uint8_t s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
         (((((g * s1) >> 8) + s2) * v1) & 0xff00) |
         (((((b * s1) >> 8) + s2) * v1) >> 8);
}

void led_strip_animate()
{
	static int tick = 0;
	float n1, n2, n3, n3_raw;

	for (int i=0; i < N_LEDS; i++) {
		// Range approximately -1 .. +1
		n1 = pnoise2d(3.1 * i, 0.002 * tick, 0.7, 3, seed1);
		n2 = pnoise2d(7.4 * i, 0.002 * tick, 0.7, 3, seed2);
		n3_raw = pnoise2d(8.9 * i, 0.004 * tick, 0.7, 3, seed3);

		// Color: Shift range to ~ 0 .. +1
		n1 = n1 / 2.0 + 0.5;

		// Saturation: The more positive offset, the more colorful
		n2 = n2 + 0.8;
		n2 = limit(0.0, n2, 1.0);

		// Brightness: negative offset = less likely to light up a LED
		n3_raw = (n3_raw + (g_intensity - 0.75) * 2.0) * 3.0;
		n3 = limit(0.0, n3_raw, 1.0);

		pixels[i] = colorHSV(n1 * 0xFFFF, n2 * 0xFF, n3 * 0xFF);
	}
	led_strip_update(pixels);
	// log_i(
	// 	"%6d: %6.3f, %6.3f, %6.3f (%6.3f)\n",
	// 	tick, n1, n2, n3, n3_raw
	// );

	tick++;
}
