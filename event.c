#include <LPC11xx.h>
#include <string.h>
#include "event.h"

#undef _DBGOUT
#if defined( _DBGOUT )
// timer1 задействован для того, чтобы можно было проанализировать 
// время нахождения внутри события event().
// Полезно для анализа, нет ли "передержки" внутри событий,
// при наличии "передержек", миллисекундный счетчик может начать отставать.
#include "timer1.h"
#include "dbg.h"
#else
#endif

extern uint32_t SystemCoreClock; 
#if (EVENT_64BIT == 1)
static volatile uint64_t msTicks;
#else
static volatile uint32_t msTicks;
#endif
static timer_event_t timer_events[MAX_EVENT];

void SysTick_Handler(void) {
	int i;
	timer_event_f pfunc;
	msTicks += 1000/EVENTS_FREQ;
	for (i=0; i<MAX_EVENT; i++) {
		if (timer_events[i].pfunc != NULL) {
			if (timer_events[i].cmp_val <= msTicks) {
#if defined( _DBGOUT )
				uint32_t t_start = timer1_get();
				uint32_t time;
#endif
				pfunc = timer_events[i].pfunc;
				timer_events[i].pfunc = NULL;
				pfunc();
#if defined( _DBGOUT )
				// Время работы в mks:
				time = (timer1_get() - t_start)/(TIMER_FREQ/1000000);
				if (time > (1000/EVENTS_FREQ/2)*1000) {
					// Время работы функции больше половинного интервала!
					DBG("event(%d) %dmks\r\n", i, time);
				}
#endif
			}
		}
	}
}

void event_init(void) {
	msTicks = 0;
	memset(timer_events, 0, sizeof(timer_events));
	if (SysTick_Config(SystemCoreClock / EVENTS_FREQ)) {
#if defined( _DBGOUT )
		DBG("event_init(): incorrerct SystemCoreClock, SysTick_Config() fault!\r\n");
#endif
		while (1);
	}
}

int event_set(timer_event_f pfunc, uint32_t delay) {
	int slot;
	for (slot=0; slot<MAX_EVENT; slot++) {
		if (timer_events[slot].pfunc == NULL) {
			if (delay < 1) {
				delay = 1;
			}
			timer_events[slot].cmp_val = (msTicks + delay);
			timer_events[slot].pfunc = (timer_event_f)pfunc;
			return 0;
		}
	}
	return 1;
}

int event_unset(timer_event_f pfunc) {
	int slot;
	for (slot=0; slot<MAX_EVENT; slot++) {
		if (timer_events[slot].pfunc == pfunc) {
			timer_events[slot].pfunc = NULL;
			return 0;
		}
	}
	return 1;
}

void delay_ms(uint32_t msDelay) {
	uint32_t msStart = msTicks;
	while ((msTicks-msStart) < msDelay) __WFI();
}

uint32_t get_ms_timer(void) {
#if (EVENT_64BIT == 1)
	return (uint32_t)msTicks;
#else
	return msTicks;
#endif
}

#if (EVENT_64BIT == 1)
uint64_t get_ms_timer64(void) {
	return msTicks;
}
#endif
