// Velogen specific hardware
#ifndef VELOGEN_H
#define VELOGEN_H

#define WHEEL_C 2155  // wheel circumference in [mm]
#define WHEEL_POLES 13  // number of pulses / revolution

// GPIO Pin definitions
#define P_DYN GPIO_NUM_16  // Dynamo on / off
#define P_AC GPIO_NUM_15  // N zero crossings per rotation

extern unsigned g_wheelCnt;
extern float g_speed;

// initalize velogen hardware
void velogen_init();

// call in main loop, if a button is released, sets the corresponding bit
// in return value
unsigned touch_read();

// accumulate wheel rotations in g_wheelCnt
int counter_read();

#endif
