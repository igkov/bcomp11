/*
	elog.c

	Модуль для периодичной отправки во внешнюю среду ключевых параметров 
	по текущему состоянию автомобиля. Сделано для механизма логгирования 
	данных системы. Для записи лога требуется внешнее UART-устройство.

	Отправляет следующий поток данных на выход:
	time;speed;rpm;trans;batt;dfuel;t_eng;t_akpp;t_ext;dist;gdist;mil;\r\n
	1234567891;56;1560;D3;13.8;3.86;79;95;24;13847;15483337;0;\r\n

	Где:
	 0 - time     - Метка времени (время работы двигателя в секундах).
	 1 - speed    - Скорость (км/ч).
	 2 - rpm      - Обороты двигателя (об/мин).
	 3 - trans    - Передача трансмиссии (PRND12345).
	 4 - batt     - Бортовое напряжение (В).
	 5 - fuel     - Уровень топлива в баке (л).
	 6 - dfuel    - Израсходованное за сеанс топливо (л).
	 7 - lon      - Широта.
	 8 - lat	  - Долгота.
	 9 - gtime	  - Время GPS в формате HH-MM-SS.
	10 - gdate    - Дата GPS в формате DD-MM-YYYY.
	11 - t_eng    - Температура двигателя (°C).
	12 - t_akpp   - Температура коробки передач (°C).
	13 - t_ext    - Температура наружного воздуха (°C).
	14 - p_fuel   - Давление в топливной рейке (МПа)
	15 - p_intake - Давление во впускном коллекторе (кПа)
	16 - dist     - Счетчик пробега за сеанс (м).
	17 - gdist    - Счетчик пробега за все время (м).
	18 - mil      - Флаги ошибки двигателя (0 - нет ошибок, 1 - есть активная ошибка).
	
	igorkov / 2016-2017 / igorkov.org/bcomp11v2
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "event.h"
#include "uart0.h"
#include "bcomp.h"
#include "dbg.h"
#include "errors.h"

#include "elog.h"

#define ELOG_VIN_FLAG   0x01
#define ELOG_MIL_FLAG   0x02
#define ELOG_START_FLAG 0x04

static uint8_t elog_flags = 0;
static char elog_str[128];

// Стартовая строка, используется для дальнейшей привязки строк к значениям в софте обработки:
// COLUMN:                0    1     2   3     4    5     6     7   8   9     10    11    12     13    14     15    16   17    18  
const char elog_info[] = "time;speed;rpm;trans;batt;fuel; dfuel;lon;lat;gtime;gdate;t_eng;t_akpp;t_ext;p_fuel;p_air;dist;gdist;mil;\r\n";

void elog_proc(void) {
	int offset;
	// Первая строка лога:
	if ((elog_flags & ELOG_START_FLAG) == 0) {
		// Посылаем единожды:
		elog_flags |= ELOG_START_FLAG;
		// Выставляем данные на асинхронную отправку:
		memcpy(elog_str, elog_info, strlen(elog_info));
		goto elog_send;
	}
	// Однократная отправка VIN-номера:
	if ((elog_flags & ELOG_VIN_FLAG) == 0 &&
		bcomp.vin[0] != 0) {
		elog_flags |= ELOG_VIN_FLAG;
		// Выставляем данные на асинхронную отправку VIN:
		_sprintf(elog_str, "VIN;%s\r\n", bcomp.vin);
		// Переходим на отправку:
		goto elog_send;
	}
	// Если есть флаг ошибки, пишем ошибку в лог:
	if (bcomp.mil) {
		if ((elog_flags & ELOG_MIL_FLAG) == 0) {
			// Текстовый описатель ошибки:
			char error_code[8];
			// Счетчик входов:
			elog_flags |= ELOG_MIL_FLAG;
			// Расшифровываем ошибку.
			error_decrypt(bcomp.e_code, error_code);
			// Выставляем данные на асинхронную отправку VIN:
			_sprintf(elog_str, "ERROR;%s\r\n", error_code);
			// Переходим на отправку:
			goto elog_send;
		}
	} else {
		// Если ошибка сброшена, сбрасываем и флаг:
		if (bcomp.mil == 0) {
			elog_flags &= ~ELOG_MIL_FLAG;
		}
	}
	// Конструируем посылку:
	//  0 - time   - Метка времени в UNIX-формате (если имеется, иначе возвращаем счетчик секунд).
	offset = 0;
	_sprintf(&elog_str[offset], "%d;", bcomp.utime?bcomp.utime:bcomp.time);
	//  1 - speed  - Скорость (км/ч).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.speed);
	//  2 - rpm    - Обороты двигателя (об/мин).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.rpms);
	//  3 - trans  - Передача трансмиссии (PRND12345).
	offset = strlen(elog_str);
	switch(bcomp.at_drive) {
	case 0x00: _sprintf(&elog_str[offset], "N;");  break;
	case 0x01: _sprintf(&elog_str[offset], "D1;");  break;
	case 0x02: _sprintf(&elog_str[offset], "D2;");  break;
	case 0x03: _sprintf(&elog_str[offset], "D3;");  break;
	case 0x04: _sprintf(&elog_str[offset], "D4;");  break;
	case 0x05: _sprintf(&elog_str[offset], "D5;");  break;
	case 0x0b: _sprintf(&elog_str[offset], "R;");   break;
	case 0x0d: _sprintf(&elog_str[offset], "P;");   break;
	default:   _sprintf(&elog_str[offset], "UNK;"); break;
	}
	//  4 - batt   - Бортовое напряжение (В).
	offset = strlen(elog_str);
	if (isnan(bcomp.v_ecu)) {
		_sprintf(&elog_str[offset], "0.0;");
	} else {
		_sprintf(&elog_str[offset], "%d.%d;", (int)bcomp.v_ecu, (int)(bcomp.v_ecu*10)%10);
	}
	//  5 - dfuel  - Израсходованное за сеанс топливо (л).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d.%02d;", (int)bcomp.fuel_level, (int)(bcomp.fuel_level*100)%100);
	//  6 - dfuel  - Израсходованное за сеанс топливо (л).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d.%02d;", (int)bcomp.fuel, (int)(bcomp.fuel*100)%100);
	// GPS block:
	offset = strlen(elog_str);
	if (bcomp.g_correct) {
		//  7 - lon
		_sprintf(&elog_str[offset], "%s;", bcomp.gps_val_lon);
		//	8 - lat
		offset = strlen(elog_str);
		_sprintf(&elog_str[offset], "%s;", bcomp.gps_val_lat);
		//	9 - gtime
		offset = strlen(elog_str);
		_sprintf(&elog_str[offset], "%s;", bcomp.gps_val_time);
		//  10 - gdate
		offset = strlen(elog_str);
		_sprintf(&elog_str[offset], "%s;", bcomp.gps_val_date);
	} else {
		// 7,8,9,10 - nop
		_sprintf(&elog_str[offset], ";;;;");
	}	
	// 11 - t_eng  - Температура двигателя (°C).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.t_engine);
	// 12 - t_akpp - Температура коробки передач (°C).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.t_akpp);
	// 13 - t_ext  - Температура наружного воздуха (°C).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.t_ext);
	// 14 - p_fuel
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.p_fuel);
	// 15 - p_intake
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.p_intake);
	// 16 - dist   - Счетчик пробега за сеанс (м).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.dist);
	// 17 - gdist  - Счетчик пробега за все время (м).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.moto_dist);
	// 18 - mil    - Флаги ошибки двигателя (0 - нет ошибок, 1 - есть активная ошибка).
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "%d;", (int)bcomp.mil);
	// string end:
	offset = strlen(elog_str);
	_sprintf(&elog_str[offset], "\r\n");
elog_send:
	// Выставляем данные на асинхронную отправку:
	uart0_puts(elog_str);
}

