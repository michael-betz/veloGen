#include "driver/i2c.h"
#include "esp_log.h"
#include "ina219.h"

#define I2C_ADDR 0x40
#define R_SHUNT 50000 // [uOhm]

static const char *T = "INA2019";

static uint16_t inaCfg = 0;

static void w(uint8_t reg, uint16_t val)
{
    i2c_cmd_handle_t cl = i2c_cmd_link_create();
    i2c_master_start(cl);
    i2c_master_write_byte(cl, (I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cl, reg, true);
    i2c_master_write_byte(cl, val >> 8, true);
    i2c_master_write_byte(cl, val & 0xFF, true);
    i2c_master_stop(cl);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cl, 10 / portTICK_RATE_MS);
    if (ret != ESP_OK)
        ESP_LOGE(T, "w failed %x", ret);
    i2c_cmd_link_delete(cl);
}

static void setcfg(unsigned shift, unsigned n_bits, unsigned val)
{
    unsigned mask = (1 << n_bits) - 1;
    mask <<= shift;
    val <<= shift;
    inaCfg &= ~mask;
    inaCfg |= (val & mask);
    w(0, inaCfg);
}

static uint16_t rui(uint8_t reg)
{
    uint8_t buf[2];

    i2c_cmd_handle_t cl = i2c_cmd_link_create();
    i2c_master_start(cl);
    i2c_master_write_byte(cl, (I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cl, reg, true);

    i2c_master_start(cl);
    i2c_master_write_byte(cl, (I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cl, buf, 2, false);
    i2c_master_stop(cl);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cl, 10 / portTICK_RATE_MS);
    if (ret != ESP_OK)
        ESP_LOGE(T, "rui failed %x", ret);
    i2c_cmd_link_delete(cl);

    return (buf[0] << 8 | buf[1]);
}

static int16_t ri(uint8_t reg) { return (int16_t)rui(reg); }

void inaInit()
{
    w(0, 1 << 15);  // reset
    inaCfg = rui(0);
}

// current in [mA]
int inaI()
{
    int tmp = ri(1);
    return (tmp * 10 * 1000 / R_SHUNT);
}

// bus voltage in [mV]
int inaV()
{
    int tmp = ri(2) & 0xFFF8;
    return (tmp / 2);
}

// averaging factor: 0 - 7
// applies to BUS and SHUNT
// always 12 bit mode
void inaAvg(unsigned val)
{
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
