#include <stdint.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "event.h"
#include "dbg.h"

static volatile uint32_t msTicks;
static timer_event_t timer_events[MAX_EVENT];

DWORD WINAPI event_handler(LPVOID par) {
	int i;
	timer_event_f pfunc;
	while (1) {
		msTicks = GetTickCount();
		for (i=0; i<MAX_EVENT; i++) {
			if (timer_events[i].pfunc != NULL) {
				if (timer_events[i].cmp_val <= msTicks) {
					pfunc = timer_events[i].pfunc;
					timer_events[i].pfunc = NULL;
					pfunc();
				}
			}
		}
		Sleep(1);
	}
}

void event_init(void) {
	HANDLE hInstance;
	HANDLE hThread;
	DWORD dwThreadId;
	
	msTicks = 0;
	memset(timer_events, 0, sizeof(timer_events));
	// todo: init win32 thread!
	// Инициализируем пток для обработчика окна:
	hInstance = GetModuleHandle(NULL);
	hThread = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			event_handler,          // thread function name
			NULL,                   // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);           // returns the thread identifier 
	if (hThread == NULL) {
		MessageBox(NULL, "Не могу создать поток!", "Ошибка", MB_OK);
		return;
	}
}

#if 0
int event_set(uint32_t slot, timer_event_f pfunc, uint32_t delay) {
	if (slot >= MAX_EVENT) {
		return 1;
	}
	if (delay < 1) {
		delay = 1;
	}
	timer_events[slot].cmp_val = (msTicks + delay);
	timer_events[slot].pfunc = (timer_event_f)pfunc;
	return 0;
}

int event_unset(uint32_t slot) {
	if (slot >= MAX_EVENT) {
		return 1;
	}
	timer_events[slot].pfunc = NULL;
	return 0;
}
#else
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
#endif

void delay_ms(uint32_t msDelay) {
	uint32_t msStart = GetTickCount();
	while ((GetTickCount()-msStart) < msDelay) Sleep(1);
}

uint32_t get_ms_timer(void) {
	return GetTickCount();
}
