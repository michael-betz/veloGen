#ifndef INA219_H
#define INA219_H

// reset chip
void inaInit();

// read shunt current in [mA]
int inaI();

// read bus voltage in [mV]
int inaV();

// averaging factor: 0 - 7
// applies to BUS and SHUNT
// always 12 bit mode
void inaAvg(unsigned val);

// set bus voltage scaling
void inaBus32(bool val);

// set shunt voltage gain
// val = 0 (+- 40 mV), 1 (+- 80 mV), 2 (+- 160 mV) or 3 (+- 320 mV)
void inaPga(unsigned val);

// enable power safe mode
void inaOff();

// wake up from power safe mode
void inaOn();

#endif
