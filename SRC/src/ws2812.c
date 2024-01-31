#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "velo.h"
#include "json_settings.h"
#include "ws2812.h"

static const char *TAG = "ws2812";

led_strip_handle_t led_strip;
static int ws2812_intensity = 0x80;


void ws2812_off()
{
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
}

void ws2812_init(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = P_EN0,   // The GPIO that connected to the LED strip's data line
        .max_leds = N_LEDS,        // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812,            // LED strip model
        .flags.invert_out = false,                // whether to invert the output signal
    };

    // LED strip backend configuration: SPI
    led_strip_spi_config_t spi_config = {
        .clk_src = SPI_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
        .flags.with_dma = true,         // Using DMA can improve performance and help drive more LEDs
        .spi_bus = SPI2_HOST,           // SPI bus ID
    };

    // LED Strip object handle
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with SPI backend");
    ws2812_off();

    cJSON *s = getSettings();
    ws2812_intensity = jGetI(s, "strip_intensity", 0x80);
    if (ws2812_intensity > 0xFF)
        ws2812_intensity = 0xFF;
}

static unsigned triangle(unsigned x, unsigned x_max)
{
    unsigned tmp = x % (2 * x_max);
    return tmp < x_max ? tmp : 2 * x_max - tmp;
}

#define SPAKLE 0xFA000000

static void ani0(int tick)
{
    // red pulsating
    int b_min = ws2812_intensity / 2;
    int b = triangle(tick, b_min - 1);
    for (int i = 0; i < N_LEDS; i++) {
        int tmp = (random() > SPAKLE) ? 0xFF : b_min + b;
        led_strip_set_pixel(led_strip, i, tmp, 0, 0);
    }
}

static void ani1(int tick)
{
    // purple / pink rainbow
    for (int i = 0; i < N_LEDS; i++) {
        if (random() > SPAKLE)
            led_strip_set_pixel(led_strip, i, 0xFF, 0, 0);
        else
            led_strip_set_pixel_hsv(
                led_strip,
                i,
                triangle(i * 20 + tick, 130) + 270,
                0xFF,
                ws2812_intensity
            );
    }
}

void ws2812_animate()
{
    static int tick = 0;

    if (g_speed < 50)
        ani0(tick);
    else
        ani1(tick);

    led_strip_refresh(led_strip);
    tick++;
}
