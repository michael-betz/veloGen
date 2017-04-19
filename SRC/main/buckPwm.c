/*
 * buckPwm.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michael
 */

#include "driver/ledc.h"
#include "main.h"
#include "buckPwm.h"


void initPwm()
{
	//configure timer1 for high speed channels
	ledc_timer_config_t ledc_timer = {
		.bit_num = BUCK_PWM_BITS, 			//set timer counter bit number
		.freq_hz = BUCK_PWM_FREQ,      		//set frequency of pwm
		.speed_mode = LEDC_HIGH_SPEED_MODE, //timer mode,
		.timer_num = LEDC_TIMER_1    		//timer index
	};
   ESP_ERROR_CHECK( ledc_timer_config(&ledc_timer) );
   //set the configuration for BUCK
   ledc_channel_config_t pwmConfig = {
		//set LEDC channel 0
		.channel = LEDC_CHANNEL_0,
		//set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
		.duty = 0,
		//GPIO number
		.gpio_num = BUCK_GPIO,
		//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
		.intr_type = LEDC_INTR_DISABLE,
		//set LEDC mode, from ledc_mode_t
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		//set LEDC timer source, if different channel use one timer,
		//the frequency and bit_num of these channels should be the same
		.timer_sel = LEDC_TIMER_1,
	};
    ESP_ERROR_CHECK( ledc_channel_config(&pwmConfig) );
	//set the configuration for LED
//    pwmConfig.channel = LEDC_CHANNEL_1;
//    pwmConfig.gpio_num = LED_GPIO;
//	ESP_ERROR_CHECK( ledc_channel_config(&pwmConfig) );
}

void setPwm( uint16_t pwmValue )
{
	ESP_ERROR_CHECK( ledc_set_duty(    LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, pwmValue ) );
	ESP_ERROR_CHECK( ledc_update_duty( LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 ) );
//	ESP_ERROR_CHECK( ledc_set_duty(    LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, pwmValue ) );
//	ESP_ERROR_CHECK( ledc_update_duty( LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1 ) );
}

