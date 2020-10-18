#include "Arduino.h"
#include "Wire.h"
#include "ina219.h"

#define I2C_ADDR 0x40
#define R_SHUNT 50000 // [uOhm]

static uint16_t inaCfg = 0;

static void w(uint8_t reg, uint16_t val)
{
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(reg);
    Wire.write(val >> 8);
    Wire.write(val & 0xFF);
    Wire.endTransmission(true);
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
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);  // Generate repeated start with false
    Wire.readTransmission(I2C_ADDR, buf, 2);
    Wire.endTransmission(true);
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
