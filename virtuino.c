/*
	virtuino.c
	
	Прослойка-драйвер для работы с приложением Virtuino на Андроиде. 
	Предполагается его использование вместо драйвера elog.c.
	Модули Bluetooth могут использоваться любые, которые обеспечивают 
	профиль SPP и UART-интерфейс (к примеру, RN-42 или HC-05).
	
	igorkov.org / Igor Kovalenko / 2018
 */
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "virtuino.h"
#include "bcomp.h"
#include "uart0.h"
#include "dbg.h"

static char virtuino_str[256];

typedef struct {
	uint8_t id;
	uint8_t type;
	char cmd;
	uint8_t res;
	void *value;
} virtuino_unit_t, *pvirtuino_unit_t;

#define VIRTUINO_TYPE_INT     (0)
#define VIRTUINO_TYPE_BYTE    (1)
#define VIRTUINO_TYPE_FLOAT   (2)
#define VIRTUINO_TYPE_DOUBLE  (3)
#define VIRTUINO_TYPE_STRING  (4)

//
// Структура-описание данных на отправку:
//
const static virtuino_unit_t units[] = {
	{ 1, VIRTUINO_TYPE_INT,    'V', 0, &bcomp.time },
	{ 2, VIRTUINO_TYPE_INT,    'V', 0, &bcomp.speed },
	{ 3, VIRTUINO_TYPE_INT,    'V', 0, &bcomp.rpm },
	{ 4, VIRTUINO_TYPE_INT,    'V', 0, &bcomp.t_engine },
	{ 5, VIRTUINO_TYPE_INT,    'V', 0, &bcomp.t_akpp },
	{ 6, VIRTUINO_TYPE_INT,    'V', 0, &bcomp.t_engine },
	{ 7, VIRTUINO_TYPE_STRING, 'T', 0, &bcomp.vin },
};

static void virtuino_unit_get(const virtuino_unit_t *punit, char *strout) {
	switch (punit->type) {
	case VIRTUINO_TYPE_INT:
		_sprintf(strout, "!%c%02d=%d$", punit->cmd, punit->id, *(int*)punit->value);
		break;
	case VIRTUINO_TYPE_BYTE:
		_sprintf(strout, "!%c%02d=%d$", punit->cmd, punit->id, *(char*)punit->value);
		break;
	case VIRTUINO_TYPE_FLOAT:
		if (isnan(*(float*)punit->value)) {
			_sprintf(strout, "!%c%02d=NAN$", punit->cmd, punit->id);
		} else {
			_sprintf(strout, "!%c%02d=%d.%02d$", punit->cmd, punit->id, (int)*(float*)punit->value, 
				(int)(*(float*)punit->value * 100) % 100);
		}
		break;
	case VIRTUINO_TYPE_DOUBLE:
		if (isnan(*(double*)punit->value)) {
			_sprintf(strout, "!%c%02d=NAN$", punit->cmd, punit->id);
		} else {
			_sprintf(strout, "!%c%02d=%d.%02d$", punit->cmd, punit->id, (int)*(double*)punit->value, 
				(int)(*(double*)punit->value * 100) % 100);
		}
		break;
	case VIRTUINO_TYPE_STRING:
		_sprintf(strout, "!%c%02d=%s$", punit->cmd, punit->id, *(char*)punit->value);
		break;
	default:
		DBG("virtuino_unit_get(): unknown value type (%d)!\r\n", punit->type);
	}
}

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
