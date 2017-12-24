#include <LPC11xx.h>
#include <stdint.h>
#include <string.h>

#include "event.h"

volatile uint32_t msTicks;
timer_event_t timer_events[MAX_EVENT];

void SysTick_Handler(void) {
	int i;
	timer_event_f pfunc;
	msTicks += 5;
	for (i=0; i<MAX_EVENT; i++)
		if (timer_events[i].pfunc != NULL)
			if ((timer_events[i].cmp_val - msTicks) > 0x80000000) {
				pfunc = timer_events[i].pfunc;
				timer_events[i].pfunc = NULL;
				pfunc();
			}
}

void event_init(void) {
	msTicks = 0;
	memset(timer_events, 0, sizeof(timer_events));
	if (SysTick_Config(SystemCoreClock / (1000 / 5))) // Init SysTick 5ms
		while (1);
}

int event_set(uint32_t slot, timer_event_f pfunc, uint32_t delay) {
	if (slot >= MAX_EVENT) while(1);
	timer_events[slot].cmp_val = (msTicks + delay);
	timer_events[slot].pfunc = (timer_event_f)pfunc;
	return 0;
}

int event_unset(uint32_t slot) {
	if (slot >= MAX_EVENT) while(1);
//	timer_events[slot].cmp_val = 0;
	timer_events[slot].pfunc = NULL;
	return 0;
}

void delay_ms(uint32_t msDelay) {
	uint32_t msStart = msTicks;
	while ((msTicks-msStart) < msDelay) __WFI();
}

uint32_t get_ms_timer(void) {
	return msTicks;
}
