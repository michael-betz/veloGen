/*
 * buckPwm.h
 *
 *  Created on: Apr 5, 2017
 *      Author: michael
 */

#ifndef MAIN_BUCKPWM_H_
#define MAIN_BUCKPWM_H_

#define BUCK_PWM_FREQ 500000		//[Hz]
#define BUCK_PWM_BITS 7
#define BUCK_PWM_MAX ((1<<BUCK_PWM_BITS)-1)	// MAx duty for high side driver startup: BUCK_PWM_MAX-13

void initPwm();
void setPwm( uint16_t pwmValue );



#endif /* MAIN_BUCKPWM_H_ */
