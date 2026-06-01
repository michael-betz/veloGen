#include "ina219.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <string.h>

#define I2C_ADDR 0x40
#define R_SHUNT 50000  // [uOhm]

static i2c_master_bus_handle_t bus_handle = 0;
static i2c_master_dev_handle_t dev_handle = 0;

static const char *T = "INA219";

static uint16_t inaCfg = 0;

static void w(uint8_t reg, uint16_t val) {
    const uint8_t buf[3] = {reg, val >> 8, val & 0xFF};
    esp_err_t ret = i2c_master_transmit(dev_handle, buf, 3, 100);
    if (ret != ESP_OK)
        ESP_LOGE(T, "w failed %x", ret);
}

static void setcfg(unsigned shift, unsigned n_bits, unsigned val) {
    unsigned mask = (1 << n_bits) - 1;
    mask <<= shift;
    val <<= shift;
    inaCfg &= ~mask;
    inaCfg |= (val & mask);
    w(0, inaCfg);
}

static uint16_t rui(uint8_t reg) {
    uint8_t rbuf[2] = {0};

    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg, 1, rbuf, 2, 100);
    if (ret != ESP_OK)
        ESP_LOGE(T, "rui failed %x", ret);

    return (rbuf[0] << 8 | rbuf[1]);
}

static int16_t ri(uint8_t reg) { return (int16_t)rui(reg); }

void inaInit() {
    if (dev_handle != 0) {
        i2c_master_bus_rm_device(dev_handle);
        dev_handle = 0;
    }

    if (bus_handle != 0) {
        i2c_del_master_bus(bus_handle);
        bus_handle = 0;
    }

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_12,
        .scl_io_num = GPIO_NUM_14,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
        .flags.allow_pd = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    w(0, 1 << 15);  // reset
    inaCfg = rui(0);
}

// current in [mA]
int inaI() {
    int tmp = ri(1);
    return (tmp * 10 * 1000 / R_SHUNT);
}

// bus voltage in [mV]
int inaV() {
    int tmp = ri(2) & 0xFFF8;
    return (tmp / 2);
}

// averaging factor: 0 - 7
// applies to BUS and SHUNT
// always 12 bit mode
void inaAvg(unsigned val) {
    val |= 8;
    setcfg(3, 4, val);
    setcfg(7, 4, val);
}

// set bus voltage scaling
// val = false: 16 V FSR, true: 32 V FSR
void inaBus32(bool val) { setcfg(13, 1, val); }

// set shunt voltage divider
// val = 0: +- 40 mV FSR, 1: +- 80 mV FSR, 2: +- 160 mV FSR, 3: +- 320 mV FSR
void inaPga(unsigned val) { setcfg(11, 2, val); }

// enable power safe mode
void inaOff() { w(0, inaCfg & 0xFFF8); }

// wake up from power safe mode
void inaOn() { w(0, inaCfg); }
