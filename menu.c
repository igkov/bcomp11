#include <stdint.h>
#include <string.h>
#include <math.h>
#include "event.h"
#include "buttons.h"
#include "dbg.h"
#include "graph.h"
#include "bcomp.h"

#include "menu.h"

const menu_t menu[] = {
	{    0, MENU_FLOAT, 1, 0, "Акк.макс",    "В",   &bcomp.setup.v_max,    1, 130, 160 },
	{    1, MENU_FLOAT, 1, 0, "Акк.мин",     "В",   &bcomp.setup.v_min,    1, 105, 130 },
	{    2, MENU_INT,   0, 0, "Коробка",     "°C",  &bcomp.setup.t_at,     1, 90, 130 },
	{    3, MENU_INT,   0, 0, "Двигатель",   "°C",  &bcomp.setup.t_eng,    1, 70, 120 },
	//{    4, MENU_FLAG,  0, 0, "Дат.топл.",   "",    &bcomp.setup.f_fuel,   1, 0, 1 },
	//{    5, MENU_FLOAT, 1, 0, "Ур.топл.",    "Л",   &bcomp.setup.l_fuel,   5, 10, 100 },
	//{    6, MENU_INT,   0, 0, "Час.пояс",    "ч",   &bcomp.setup.time,     1, -12, 12 },
	{    7, MENU_INT,   0, 0, "Повт.пред.",  "мин", &bcomp.setup.w_delay,  5, 0, 240 },
	{    8, MENU_FLAG,  0, 0, "Наруж.дат.",  "",    &bcomp.setup.f_ext,    1, 0, 1 },
	{    9, MENU_FLAG,  0, 0, "Нар.пред.",   "",    &bcomp.setup.f_ext_w,  1, 0, 1 },
	{   10, MENU_INT,   0, 0, "Наруж.тем.",  "°C",  &bcomp.setup.t_ext,    1, -10, 10 },
	{   12, MENU_FLAG,  0, 0, "GPS",         "",    &bcomp.setup.f_gps,    1, 0, 1 },
	//{   13, MENU_INT,   0, 0, "GPS скор.",   "",    &bcomp.setup.i_gps,    4800, 4800, 115200 },
	{   14, MENU_FLAG,  0, 0, "ESP руль",    "",    &bcomp.setup.f_esp,    1, 0, 1 },
	{   15, MENU_FLOAT, 2, 0, "Калиб.топ.",  "",    &bcomp.setup.fuel_cal, 1, 300, 400 },
	{   16, MENU_FLOAT, 2, 0, "Калиб.ск.",   "",    &bcomp.setup.speed_cal,1, 80, 120 },
	{   17, MENU_FLAG,  0, 0, "Лог.дан.",    "",    &bcomp.setup.f_log,    1, 0, 1 },
	{   18, MENU_INT,   2, 0, "Контраст",    "",    &bcomp.setup.contrast, 4, 1, 255 },
	// ID ниже нельзя менять, они задействованы в логике уровнем выше! Порядок не важен.
	{ 0xF0, MENU_OTHER, 0, 0, "Инфо",        "",    (void*)0,              0, 0, 0 },
	{ 0xF2, MENU_OTHER, 0, 0, "VIN",         "",    (void*)0,              0, 0, 0 },
	{ 0xF3, MENU_OTHER, 0, 0, "Пробег",      "",    (void*)0,              0, 0, 0 },
	{ 0xF1, MENU_OTHER, 0, 0, "Сохранить",   "",    (void*)0,              0, 0, 0 },
	{ 0xFF, MENU_EXIT,  0, 0, "Выход",       "",    (void*)0,              0, 0, 0 },
};

static char menu_inside = 0;
int menu_work(int *act) {
	static int id = 0;
	int exp;
	char str[16];

	if (menu[id].exp == 0) {
		exp = 1;
	} else
	if (menu[id].exp == 1) {
		exp = 10;
	} else
	if (menu[id].exp == 2) {
		exp = 100;
	} else {
		exp = 1;
	}

	if (*act & BUTT_SW1) {
		if (menu_inside) {
			if (menu[id].type == MENU_OTHER) {
				//menu_inside = 0;
			} else 
			if (menu[id].type == MENU_FLAG ||
				menu[id].type == MENU_INT) {
				*(int*)menu[id].value += menu[id].step;
				if (*(int*)menu[id].value > menu[id].max) {
					*(int*)menu[id].value = menu[id].min;
				}
			} else
			if (menu[id].type == MENU_FLOAT) {
				*(float*)menu[id].value += ((float)menu[id].step)/exp;
				if (*(float*)menu[id].value > ((float)menu[id].max)/exp) {
					*(float*)menu[id].value = ((float)menu[id].min)/exp;
				}
			}
		} else {
			id++;
			id = id%(sizeof(menu)/sizeof(menu_t));
		}
	}
	if (*act & BUTT_SW2) {
		if (menu_inside) {
			if (menu[id].type == MENU_OTHER) {
				//menu_inside = 0;
			} else 
			if (menu[id].type == MENU_FLAG ||
				menu[id].type == MENU_INT) {
				*(int*)menu[id].value -= menu[id].step;
				if (*(int*)menu[id].value < menu[id].min) {
					*(int*)menu[id].value = menu[id].max;
				}
			} else
			if (menu[id].type == MENU_FLOAT) {
				*(float*)menu[id].value -= ((float)menu[id].step)/exp;
				if (*(float*)menu[id].value < ((float)menu[id].min)/exp) {
					*(float*)menu[id].value = ((float)menu[id].max)/exp;
				}
			}
		} else {
			if (id == 0) {
				id = (sizeof(menu)/sizeof(menu_t)) - 1;
			} else {
				id--;
			}
		}
	} 
	if (*act & BUTT_SW1_LONG) {
		// enter
		if (menu[id].type == MENU_EXIT) {
			id = 0;
			return 0;
		}
		// Если MENU_OTHER-пункт, резрешаем вход в него, дальнейшая обработка 
		// длинного нажатия воложена на логику экрана.
		if (menu[id].type == MENU_OTHER) {
			if (menu_inside == 0) {
				menu_inside = 1;
				*act ^= BUTT_SW1_LONG;
			}
		} else {
			menu_inside = menu_inside?0:1;
		}
	}
	if (*act & BUTT_SW2_LONG) {
		// Просто выход.
		id = 0;
		return 0;
	}
	if (menu_inside) {
		if (menu[id].type == MENU_OTHER) {
			return menu[id].id;
		} else if (menu[id].type == MENU_FLAG) {
			graph_puts16(64, 0, 1, menu[(id&0xFF)].name);
			_sprintf(str, "%s", *(int*)menu[id].value?"[Вкл.]":"[Выкл.]");
			graph_puts16(64, 24, 1, str);
		} else 
		if (menu[id].type == MENU_INT) {
			graph_puts16(64, 0, 1, menu[(id&0xFF)].name);
			_sprintf(str, "%d%s", *(int*)menu[id].value, menu[id].val);
			graph_puts16(64, 24, 1, str);
		} else 
		if (menu[id].type == MENU_FLOAT) {
			graph_puts16(64, 0, 1, menu[(id&0xFF)].name);
			if (exp == 1 || exp == 10) {
				_sprintf(str, "%d.%d%s", (int)(*(float*)menu[id].value), (int)(*(float*)menu[id].value*exp)%exp, menu[id].val);
			} else 
			if (exp == 100) {
				_sprintf(str, "%d.%02d%s", (int)(*(float*)menu[id].value), (int)(*(float*)menu[id].value*exp)%exp, menu[id].val);
			}
			graph_puts16(64, 24, 1, str);
		}
	} else {
		graph_puts16(64, 0, 1, "Меню");
		if (id > 2) {
			graph_puts16(0, 48, 0, "\x3E");
			graph_puts16(10, 16, 0, menu[id-2].name);
			graph_puts16(10, 32, 0, menu[id-1].name);
			graph_puts16(10, 48, 0, menu[id-0].name);
		} else {
			graph_puts16(0, 16+16*id, 0, "\x3E");
			graph_puts16(10, 16, 0, menu[0].name);
			graph_puts16(10, 32, 0, menu[1].name);
			graph_puts16(10, 48, 0, menu[2].name);
		}
	}
	return -1;
}

void menu_back(void) {
	menu_inside = 0;
}
