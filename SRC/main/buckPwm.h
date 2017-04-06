/*
 * buckPwm.h
 *
 *  Created on: Apr 5, 2017
 *      Author: michael
 */

#ifndef MAIN_BUCKPWM_H_
#define MAIN_BUCKPWM_H_

#define BUCK_PWM_FREQ 75000		//[Hz]

void initPwm();
void setPwm( uint16_t pwmValue );



#endif /* MAIN_BUCKPWM_H_ */
