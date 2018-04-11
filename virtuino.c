/*
	virtuino.c
	
	Прослойка-драйвер для работы с приложением Virtuino на Андроиде. 
	Предполагается его использование вместо драйвера elog.c.
	Модули Bluetooth могут использоваться любые, которые обеспечивают 
	профиль SPP и UART-интерфейс (к примеру, RN-42 или HC-05).
	
	igorkov.org / Igor Kovalenko / 2018
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "virtuino.h"
#include "bcomp.h"
#include "uart0.h"
#include "dbg.h"

typedef struct {
	uint8_t id;    // Номер канала
	uint8_t type;  // Тип переменной в микропрограмме
	uint8_t vtype; // Тип переменной в протоколе
	uint8_t res;
	void *value;
} virtuino_unit_t, *pvirtuino_unit_t;

#define VIRTUINO_TYPE_INT     (0)
#define VIRTUINO_TYPE_BYTE    (1)
#define VIRTUINO_TYPE_FLOAT   (2)
#define VIRTUINO_TYPE_DOUBLE  (3)
#define VIRTUINO_TYPE_STRING  (4)

#define VIRTUINO_PTYPE_D      (0)
#define VIRTUINO_PTYPE_V      (1)
#define VIRTUINO_PTYPE_Q      (2)

//
// Структура-описание данных на отправку:
//
const static virtuino_unit_t units[] = {
	{ 1, VIRTUINO_TYPE_INT, 'V', 0, &bcomp.time },
	{ 2, VIRTUINO_TYPE_INT, 'V', 0, &bcomp.speed },
	{ 3, VIRTUINO_TYPE_INT, 'V', 0, &bcomp.rpm },
	{ 4, VIRTUINO_TYPE_INT, 'V', 0, &bcomp.t_engine },
	{ 5, VIRTUINO_TYPE_INT, 'V', 0, &bcomp.t_akpp },
	{ 6, VIRTUINO_TYPE_INT, 'V', 0, &bcomp.t_engine },
	//{ 7, VIRTUINO_TYPE_STRING, 0, 0, &bcomp.vin },
};

static void virtuino_unit_get(const virtuino_unit_t *punit, char *strout) {
	switch (punit->type) {
	case VIRTUINO_TYPE_INT:
		_sprintf(strout, "!DV%02d=%d$", punit->id, *(int*)punit->value);
		break;
	case VIRTUINO_TYPE_BYTE:
		_sprintf(strout, "!DV%02d=%d$", punit->id, *(char*)punit->value);
		break;
	case VIRTUINO_TYPE_FLOAT:
		if (isnan(*(float*)punit->value)) {
			_sprintf(strout, "!V%02d=NAN$", punit->id);
		} else {
			_sprintf(strout, "!V%02d=%d.%02d$", punit->id, (int)*(float*)punit->value, 
				(int)(*(float*)punit->value * 100) % 100);
		}
		break;
	case VIRTUINO_TYPE_DOUBLE:
		if (isnan(*(double*)punit->value)) {
			_sprintf(strout, "!V%02d=NAN$", punit->id);
		} else {
			_sprintf(strout, "!V%02d=%d.%02d$", punit->id, (int)*(double*)punit->value, 
				(int)(*(double*)punit->value * 100) % 100);
		}
		break;
#if 0
	case VIRTUINO_TYPE_STRING:
		_sprintf(strout, "!V%02d=%s$", punit->id, *(char*)punit->value);
		break;
#endif
	default:
		DBG("virtuino_unit_get(): unknown value type (%d)!\r\n", punit->type);
	}
}

static void virtuino_unit_set(const virtuino_unit_t *punit, char *strout) {
	// TODO
}

int virtuino_unit_find(int ch, int type) {
	int i;
	for (i=0; i<sizeof(units)/sizeof(virtuino_unit_t); i++) {
		if (units[i].id == ch && units[i].vtype == type) {
			return i;
		}
	}
	return -1;
}

#if 0
// !!! WARNING !!!
// Функция работает не в рамках протокола Virtuino.
// Virtuino делает запросы параметров через определенные промежутки времени.
// Здесь же реализована отправка данных самостоятельно каждую секунду.
// Таким же образом работает elog.c модуль.
// В процессе, происходит определение команды и производятся действия/ответы.
void virtuino_proc(void) {
	int i;
	int offset = 0;
	for (i=0; i<sizeof(units)/sizeof(virtuino_unit_t); i++) {
		virtuino_unit_get(&units[i], &virtuino_str[offset]);
		offset = strlen(virtuino_str);
	}
	// Выставляем данные на асинхронную отправку:
	uart0_puts(virtuino_str);	
}
#endif

void virtuino_proc(uint8_t data) {
	static int offset = 0;
	static char cmd[64];

	int ch;
	char *tmp;
	int unit_n;

	if (offset < sizeof(cmd)) {
		cmd[offset] = data;
		offset++;
	}
		  
	if (data == '!') {
		offset = 0;
	} else if (data == '$') {
		if (offset < sizeof(cmd)) {
			cmd[offset] = '0';
			offset++;
		}
	 
		ch = atoi(&cmd[1]);
		tmp = strstr(cmd, "=");
		unit_n = virtuino_unit_find(ch, cmd[0]);
		if (unit_n == -1) {
			goto end;
		}
		// !!! PROC !!!
		switch (cmd[0]) {
		case 'C': // Команда инициализации
			//???
			break;
		case 'A': // Аналоговый вход
			
			break;
		case 'D': // Виртуальный вывод VD
			if (tmp[1] == '?') {
				virtuino_unit_get(&units[unit_n], cmd);
			} else {
				virtuino_unit_set(&units[unit_n], &tmp[1]);
			}
			break;
		case 'V': // Виртуальный вывод V
			
			break;
		case 'Q': // Цифровой вывод
			
			break;
		}
		// Выставляем данные на асинхронную отправку:
		uart0_puts(cmd);	
	}
end:
	return;
}

