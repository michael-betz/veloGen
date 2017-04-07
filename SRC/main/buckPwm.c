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
   ledc_timer_config_t ledc_timer = {
		.bit_num = LEDC_TIMER_10_BIT, 		//set timer counter bit number
		.freq_hz = BUCK_PWM_FREQ,      		//set frequency of pwm
		.speed_mode = LEDC_LOW_SPEED_MODE,  //timer mode,
		.timer_num = LEDC_TIMER_1    		//timer index
	};
    //configure timer0 for high speed channels
   ESP_ERROR_CHECK( ledc_timer_config(&ledc_timer) );
//   ESP_ERROR_CHECK( ledc_timer_set( LEDC_LOW_SPEED_MODE,  LEDC_TIMER_1, uint32_t div_num, LEDC_TIMER_10_BIT, ledc_clk_src_t clk_src) );
   ledc_channel_config_t ledc_channel = {
		//set LEDC channel 0
		.channel = LEDC_CHANNEL_0,
		//set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
		.duty = 0,
		//GPIO number
		.gpio_num = LED_GPIO,
		//GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
		.intr_type = LEDC_INTR_DISABLE,
		//set LEDC mode, from ledc_mode_t
		.speed_mode = LEDC_LOW_SPEED_MODE,
		//set LEDC timer source, if different channel use one timer,
		//the frequency and bit_num of these channels should be the same
		.timer_sel = LEDC_TIMER_1,
	};
	//set the configuration
    ESP_ERROR_CHECK( ledc_channel_config(&ledc_channel) );
}

void setPwm( uint16_t pwmValue )
{
	ESP_ERROR_CHECK( ledc_set_duty(    LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pwmValue ) );
	ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0 ) );
}

