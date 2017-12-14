/*
	nmea.c - обработка посылок от GPS. Максимально обрезанная версия.

	Дополнительная информация:
	http://home.mira.net/~gnb/gps/nmea.html

	igorkov / 2012-2017 / fsp@igorkov.org
 */
#include <stdint.h>
#include <string.h>

#include "dbg.h"
#include "nmea.h"
#include "bcomp.h"

int offset;
char nmea_string[256];

#define STROUT_SIZE 16
#define NMEA_NO_CHECK 0

#if (NMEA_NO_CHECK == 0)
static char hex2char(char *str) {
	char ret = 0;
	
	if ((unsigned)(str[0] - '0') < 10)
		ret += (str[0] - '0') << 4;
	else if (str[0] >= 'A' && str[0] <= 'F')
		ret += (str[0] - 'A' + 10) << 4;
	else if (str[0] >= 'a' && str[0] <= 'f')
		ret += (str[0] - 'a' + 10) << 4;

	if ((unsigned)(str[1] - '0') < 10)
		ret += str[1] - '0';
	else if (str[1] >= 'A' && str[1] <= 'F')
		ret += (str[1] - 'A' + 10);
	else if (str[1] >= 'a' && str[1] <= 'f')
		ret += (str[1] - 'a' + 10);
	
	return ret;
}

static int nmea_check(char *str) {
	int i = 0;
	char crc = 0x00;
	char crcc = 0x00;
	
	int fstart = 0;
	int fend = 0;
	int fcrc = 0;
	
	while (str[i] != 0) {
		if (str[i] == '*')
			fend = 1;
		if (fstart == 1 && fend == 0)
			crc ^= str[i];
		if (str[i] == '$')
			fstart = 1;
		if (fstart == 1 && 
			fend == 1 &&
			str[i] == '*') {
			crcc = hex2char(&str[i+1]);
			fcrc = 1;
		}
		i++;
	}
	if (fstart != 1 && fend != 1 && fcrc != 1) {
		DBG("nmea_check(): bad format!\r\n");
		return 1;
	}
	if (crcc != crc) {
		DBG("nmea_check(): bad crc!\r\n");
		return 2;
	}
	return 0;
}
#endif

static void nmea_get_param(char *str, int npar, char *strout) {
	int n = 0;
	int i = 0, j = 0;
	int fcp = -1;
	while (str[i] != '$' && str[i] != 0x00) {
		i++;
	}
	if (str[i] == 0x00) {
		strout[0] = 0x00;
		return;
	}
	i++;
	while (str[i] != 0x00) {
		if (str[i] == ',') {
			n++;
		}
		if (npar == n) {
			fcp = 0;
		}
		if (npar == n && str[i] != ',') {
			strout[j++] = str[i];
			if (j == STROUT_SIZE-1) {
				break;
			}
			fcp = 1;
		}
		i++;
	}
	strout[j] = 0x00;
	if (fcp == -1) {
		return;
	}
	return;
}

/*
	nmea_parce()

	Функция обработки строки GPS-модуля (формат NMEA).
 */
void nmea_parce(char *str) {
	int ret;
	int i = 0;
	char strout[STROUT_SIZE];
#if (NMEA_NO_CHECK == 0)
	// Проверка корректности посылки NMEA:
	ret = nmea_check(str);
	if (ret) {
		return;
	}
#endif
	// Проверка типа посылки (нужна RMC):
	while(str[i] != '$' && str[i] != 0x00) {
		i++;
	}
	if (str[i] == 0x00 || str[i+3] != 'R' || str[i+4] != 'M' || str[i+5] != 'C') {
		return;
	}
	// Проверка корректности навигационных данных в посылке:
	nmea_get_param(str, 2, strout);
	if (strout[0] == 'A') {
		bcomp.g_correct = 1;
	} else
	if (strout[0] == 'V') {
		bcomp.g_correct = 0;
		return;
	} else {
		return;
	}
	// Получаем время:
	nmea_get_param(str, 1, strout);
	bcomp.gtime.hour = (strout[0] - '0') * 10 + (strout[1] - '0');
	bcomp.gtime.min  = (strout[2] - '0') * 10 + (strout[3] - '0');
	bcomp.gtime.sec  = (strout[4] - '0') * 10 + (strout[5] - '0');
	// Получаем дату:
	nmea_get_param(str, 9, strout);
	bcomp.gtime.date  = (strout[0] - '0') * 10 + (strout[1] - '0');
	bcomp.gtime.month = (strout[2] - '0') * 10 + (strout[3] - '0');
	bcomp.gtime.year  = (strout[4] - '0') * 10 + (strout[5] - '0') + 2000;
	_sprintf(bcomp.gps_val_time, "%02d-%02d-%02d", bcomp.gtime.hour, bcomp.gtime.min, bcomp.gtime.sec);
	_sprintf(bcomp.gps_val_date, "%02d-%02d-%04d", bcomp.gtime.date, bcomp.gtime.month, bcomp.gtime.year);
	// Заносим метку времени:
	bcomp.utime = time_to_unix(&bcomp.gtime);
	// Получаем координаты:
	nmea_get_param(str, 4, bcomp.gps_val_lat);
	nmea_get_param(str, 3, &bcomp.gps_val_lat[1]);
	nmea_get_param(str, 6, bcomp.gps_val_lon);
	nmea_get_param(str, 5, &bcomp.gps_val_lon[1]);
	DBG("GPS:%s:%s:%s:%s\r\n", bcomp.gps_val_time, bcomp.gps_val_date, bcomp.gps_val_lat, bcomp.gps_val_lon);
	return;
}

void nmea_proc(uint8_t ch) {
	if (bcomp.setup.f_gps == 0) {
		return;
	}
	bcomp.nmea_cnt++;
	if (offset >= sizeof(nmea_string)-1 ||
		ch == '\r' || 
		ch == '\n') {
		// Вставляем конец строки
		nmea_string[offset] = 0x00;
		// Обработка принятой строки:
		if (offset > 5) { // короткие строки игнорируем.
			nmea_parce(nmea_string);
		}
		// Прием новой строки:
		offset = 0;
	} else {
		nmea_string[offset] = ch;
		offset++;
	}
}

void nmea_init(void) {
	offset = 0;
	memset(nmea_string, 0x00, sizeof(nmea_string));
}

/**
	Календарь, число дней в каждом месяце
 */
const unsigned char calendar_m[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/**
	time_to_unix()

	Функция возвращает число секунд с начала года
	Учет високосных годов до конца 21 века.
	2100 год будет не високосным, но эта функция посчитает его високосным.
 */
uint32_t time_to_unix(pgpstime_t op) {
	unsigned char i;
	unsigned short j;
	unsigned long ret = 0;
	if (op->year < 1970) return 0;
	for (j = 1970; j < op->year; j++) {
		ret += 365*24*3600;
		if ((j % 4) == 0) {
			ret += 24*3600;
		}
	}
	for (i = 0; i < op->month - 1; i++)
		ret += calendar_m[i] * 3600 * 24;
	// Учет 29 февраля. Год кратен 4ем и месяц начиная с 3его.
	if ((op->month > 2) && ((op->year % 4) == 0))
		ret += 3600 * 24;
	ret += (op->date - 1) * 3600 * 24;
	ret += op->sec + op->min * 60 + op->hour * 3600;
	return ret;
}
