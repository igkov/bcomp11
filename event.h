#ifndef __EVENT_H__
#define __EVENT_H__

// VERSION 2.0
// В версии 2.0 не требуется нумеровать номера слотов под функции. 
// Слоты выбираются программно по принципу первого свободного.

#include <stdint.h>

// Количество обрабатываемых слотов:
#define MAX_EVENT 14
// Использовать ли 64 битный счетчик миллисекунд?
// 64 битный счетчик миллисекунд требуется использовать, 
// если время работы будет превышать 40 суток. Следует учитывать, 
// что в моменты переполнении миллисекундного счетчика, обработка 
// событий будет непредсказуема.
#define EVENT_64BIT 0
// Частота обработки событий (кол-во прерываний в секунду).
// Выбирать значение, чтобы 1000/EVENTS_FREQ было целым,
// иначе временные интервалы будут считаться неправильно.
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
