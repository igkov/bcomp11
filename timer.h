#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void timer1_init(void);
void timer1_pwm_on(void);
void timer1_pwm_off(void);
void timer1_pwm_freq(int freq);
uint32_t timer1_get(void);

#endif
