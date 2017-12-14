#include "timer.h"
#include "event.h"
#include "beep.h"
#include "notes.h"

#if defined( NO_SOUND ) 
#define timer1_pwm_on()
#endif
		 
static int *pmelody;
static void event_beep(void) {
	timer1_pwm_off();
	if (*pmelody == END) {
		pmelody = (void*)0;
		return;
	} else if (*pmelody != R) {
		timer1_pwm_freq(*pmelody);
	}
	pmelody++;
	event_set(2, event_beep, *pmelody);
	pmelody++;
}

void beep_init(void) {
	timer1_init();
	pmelody = (void*)0;
}

void beep(int time, int freq) {
	timer1_pwm_on();
	timer1_pwm_freq(freq);
	delay_ms(time);
	timer1_pwm_off();
}

void beep_play(int *melody) {
	timer1_pwm_on();
	pmelody = melody;
	event_set(2, event_beep, 5);
}

int beep_is_play(void) {
	if (pmelody == (void*)0) {
		return 0;
	}
	return 1;
}

void beep_sound(int time, int freq) {
	timer1_pwm_on();
	timer1_pwm_freq(freq);
	delay_ms(time);
	timer1_pwm_off();
}
