#ifndef TOUCH_H
#define TOCUH_H

void touch_init();

// call in main loop, if a button is released, sets the corresponding bit
// in return value
unsigned touch_read();

#endif
