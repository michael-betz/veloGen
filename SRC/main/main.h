/*
 * main.h
 *
 *  Created on: Apr 5, 2017
 *      Author: michael
 */

#ifndef MAIN_H_
#define MAIN_H_



// Seems like we get 9 pulses per revolution
#define GEN_GPIO  GPIO_NUM_5
#define LED_GPIO  GPIO_NUM_12
#define BUCK_GPIO GPIO_NUM_4

// ADC (VBatt) calibration constants
#define ADC_VBATT_CHANNEL    3
#define ADC_M            97245
#define ADC_B         28531865
#define ADC_D           100000
#define ADC_GET_VBATT(cnt) ( ( (uint32_t)cnt * ADC_M + ADC_B ) / ADC_D ) //[mV]


#endif /* MAIN_H_ */
