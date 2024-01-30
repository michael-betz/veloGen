#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "velo.h"
#include "ws2812.h"

static const char *TAG = "ws2812";

led_strip_handle_t led_strip;

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
}

static unsigned triangle(unsigned x, unsigned x_max)
{
    unsigned tmp = x % (2 * x_max);
    return tmp < x_max ? tmp : 2 * x_max - tmp;
}

void ws2812_animate(uint8_t intensity)
{
    static int tick = 0;

    for (int i = 0; i < N_LEDS; i++) {
        led_strip_set_pixel_hsv(
            led_strip,
            i,
            triangle(i * 5 + tick, 120) + 260,
            0xFF,
            intensity
        );
        // ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 0xFF, 0, 0));
    }
    led_strip_refresh(led_strip);
    tick++;
}
