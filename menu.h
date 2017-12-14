#ifndef __MENU_H__
#define __MENU_H__

#include <stdint.h>

typedef struct {
	uint8_t id;
	uint8_t type;
	uint8_t exp; // Число знаков после запятой в корректировке.
	uint8_t res2;
	char name[12];
	char val[4];
	void *value;
	int step;
	int min;
	int max;
} menu_t;

#define MENU_EXIT   (0)
#define MENU_FLOAT  (1)
#define MENU_FLAG   (2)
#define MENU_INT    (3)
#define MENU_OTHER  (255)

int menu_work(int *act);
void menu_back(void);

#endif // __MENU_H__
