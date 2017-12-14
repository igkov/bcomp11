#ifndef __DBG_H__
#define __DBG_H__

/*
	dbg.h
	Отладочные функции.
	В данном модуле так же описаны прототипы.

	При этом функция _sprintf обладает особенностью:
	если в качестве указателя на строку использовано NULL - использование вывода в UART.
	если в качестве указателя на строку использовано -1 - использование вывода в OSD-устройство.
	если в качестве указателя на строку использовано -2 - использование вывода в OSD-устройство.

*/

#if defined( WIN32 )
	#include <stdio.h>
	//#define _printf printf
	//#define _sprintf sprintf
#endif

#define PRINTF_DEST_UART ((char*)0x00000000)
#define PRINTF_DEST_OSD  ((char*)0xFFFFFFFF)
#define PRINTF_DEST_LCD  ((char*)0xFFFFFFFE)
#define PRINTF_DEST_LCDF ((char*)0xFFFFFFFD)

int _printf(const char *format, ...);
int _sprintf(char *out, const char *format, ...);

void dbg_init(void);
void dbg_putchar(char ch);

#define DBG_GRAPH_X (42)
#define DBG_GRAPH_Y (6)

#ifdef _DBGOUT
	#if defined( WIN32 )
		#define DBG	  printf
		#define ASSERT(x)	if(!(x)){DBG("\nAssertion '%s' failed in %s:%s#%d!\n", #x, __FILE__, __FUNCTION__, __LINE__); while(1);}
	#else
		#define DBG   _printf
		#define ASSERT(x)	if(!(x)){DBG("\nAssertion '%s' failed in %s:%s#%d!\n", #x, __FILE__, __FUNCTION__, __LINE__); while(1);}
	#endif
#else
	#define DBG(x ...)
	#define ASSERT(x ...)
#endif

#endif
