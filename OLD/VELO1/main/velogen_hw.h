#ifndef VELOGEN_HW_H
#define VELOGEN_HW_H

#define GPIO_IBATT_SIGN 25
#define GPIO_EXT_SDA    27
#define GPIO_EXT_SCL    14
#define GPIO_LED        12
#define GPIO_SEPIC_PWM   4
#define GPIO_SEPIC_IMAX 16
#define GPIO_SPEED_PLS   2

#define ADC_CH_VGEN      0
#define ADC_CH_VBATT     3
#define ADC_CH_IBATT     6

#define DAC_VALUE        -50
#define DUTY_VALUE       50
#define SHUT_DOWN_TICKS  (15*60*1000/300)

extern RTC_DATA_ATTR uint32_t g_wheelCnt;

extern void initVelogen();
extern void adc_monitor_task(void *pvParameters);

// Maximum PWM duty cycle
void set_duty(int duty_value);

// Peak inductor current
void set_dac(int dac_value);

#endif
