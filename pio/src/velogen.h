// Velogen specific hardware
#ifndef VELOGEN_H
#define VELOGEN_H

// GPIO Pin definitions
#define P_DYN GPIO_NUM_16  // Dynamo on / off
#define P_AC GPIO_NUM_15  // N zero crossings per rotation

extern unsigned g_wheelCnt;
extern float g_speed;

// initalize velogen hardware
void velogen_init();
void velogen_loop();
void velogen_sleep();

// call in main loop, if a button is released, sets the corresponding bit
// in return value
unsigned touch_read();

// accumulate wheel rotations in g_wheelCnt
int counter_read();

#endif
