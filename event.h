#ifndef __EVENT_H__
#define __EVENT_H__

// VERSION 2.0
// � ������ 2.0 �� ��������� ���������� ������ ������ ��� �������. 
// ����� ���������� ���������� �� �������� ������� ����������.

#include <stdint.h>

// ���������� �������������� ������:
#define MAX_EVENT 14
// ������������ �� 64 ������ ������� �����������?
// 64 ������ ������� ����������� ��������� ������������, 
// ���� ����� ������ ����� ��������� 40 �����. ������� ���������, 
// ��� � ������� ������������ ��������������� ��������, ��������� 
// ������� ����� ��������������.
#define EVENT_64BIT 0
// ������� ��������� ������� (���-�� ���������� � �������).
// �������� ��������, ����� 1000/EVENTS_FREQ ���� �����,
// ����� ��������� ��������� ����� ��������� �����������.
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
