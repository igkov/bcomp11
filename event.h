#ifndef __EVENT_H__
#define __EVENT_H__

// VERSION 2.0
// ¬ версии 2.0 не требуетс€ нумеровать номера слотов под функции. 
// —лоты выбираютс€ программно по принципу первого свободного.

#include <stdint.h>

//  оличество обрабатываемых слотов:
#define MAX_EVENT 11
// »спользовать ли 64 битный счетчик миллисекунд?
// 64 битный счетчик миллисекунд требуетс€ использовать, 
// если врем€ работы будет превышать 40 суток. —ледует учитывать, 
// что в моменты переполнении миллисекундного счетчика, обработка 
// событий будет непредсказуема.
#define EVENT_64BIT 0
// „астота обработки событий (кол-во прерываний в секунду).
// ¬ыбирать значение, чтобы 1000/EVENTS_FREQ было целым,
// иначе временные интервалы будут считатьс€ неправильно.
//#define EVENTS_FREQ 1000
#define EVENTS_FREQ 200

typedef void (*timer_event_f)(void);

typedef struct {
#if (EVENT_64BIT == 1)
	uint64_t cmp_val;
#else
	uint32_t cmp_val;
#endif
	timer_event_f pfunc;
} timer_event_t, *ptimer_event_t;

void event_init(void);
int event_set(timer_event_f pfunc, uint32_t delay);
int event_unset(timer_event_f pfunc);

void delay_ms(uint32_t msDelay);
uint32_t get_ms_timer(void);
#if (EVENT_64BIT == 1)
uint64_t get_ms_timer64(void);
#endif

#endif // __EVENT_H__
