#ifndef __DRAW_H__
#define __DRAW_H__

#include <windows.h>

#define SCALE_2X 1

typedef DWORD WINAPI (*proc_f)(LPVOID);

DWORD WINAPI lcd_cpframe( uint8_t *data );
int lcd_init(proc_f fucn_addr, char *name, int size_x, int size_y);

#endif //__DRAW_H__
