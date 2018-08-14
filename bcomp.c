/*
	BCOMP11 v2 firmware
	
	Асинхронные события:
	1) button
	2) calc
	3) beep
	4) save
	5) obd
	6) analog
	7) elog
	8) warn
	
	Экраны:
	1) Основной экран с передачей и пробегом. Так же возможен вывод температуры.
	2) Температура двигателя.
	3) Температура трансмиссии (АКПП).
	4) Напряжение бортовой сети.
	5) Расходы топлива.
	6) Путь А.
	7) Путь Б.
	8) Сервисные интервалы.
	9) Положение колес.
	10) GPS-данные.
	11) Давление в топливной рейке.
	12) Давление во впускном коллекторе.
	
	Вторая итерация прошивки мини-бортового компьютера.
	Обточка функционала для реализации полноценного устройства.
	
	igorkov / 2017-2018 / igorkov.org/bcomp11v2
 */
#if defined( WIN32 )
#include <stdint.h>
#include <stdio.h>
#include "winlcd.h"
#include "windows.h"
#include "windowsx.h"
#include "string.h"
#include "Shellapi.h"
#else
#include <LPC11xx.h>
#endif
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "bcomp.h"
#include "leds.h"
#include "event.h"
#include "adc.h"
#include "analog.h"
#include "beep.h"
#include "buttons.h"
#include "dbg.h"
#include "i2c.h"
#include "eeprom.h"
#include "uart0.h"

#include "icons.h"
#include "graph.h"
#include "menu.h"
#include "wheels.h"
#include "drive.h"
#include "diagram.h"

#include "obd.h"
#include "obd_pids.h"
#include "oled128.h"
#include "config.h"
#include "errors.h"
#include "elog.h"
#include "virtuino.h"
#include "warning.h"
#include "nmea.h"

#if defined( WIN32 )
#define __WFI() Sleep(1)
#define delay_mks(a) Sleep(a/1000)
#define delay_ms(a) Sleep(a)
#endif

#define BCOMP_PAGE()        (bcomp.page & 0x0FFF)
#define BCOMP_PAGE_PREV()  (bcomp.page = ((bcomp.page & 0xF000) | ((bcomp.page - 1) & 0x0FFF)))
#define BCOMP_PAGE_NEXT()  (bcomp.page = ((bcomp.page & 0xF000) | ((bcomp.page + 1) & 0x0FFF)))

bcomp_t bcomp;
volatile uint8_t save_flag = 0;

// -----------------------------------------------------------------------------
// exeption_proc
// Попадание в эту функцию говорит о серьезной ошибке.
// -----------------------------------------------------------------------------
void exeption_proc(void) {
	DBG("exeption_proc(): while (1);\r\n");
	while (1) {
		delay_ms(300);
		led_red(1);
		delay_ms(300);
		led_red(0);
	}
}

// -----------------------------------------------------------------------------
// bcomp_XXX
// Асинхронные функции обработки различных событий.
// -----------------------------------------------------------------------------


//
// Alphabet catalog:
//
//  1 - [A] - data[3]        27 - [AA] - data[29]
//  2 - [B] - data[4]        28 - [AB] - data[30]
//  3 - [C] - data[5]        29 - [AC] - data[31]
//  4 - [D] - data[6]        30 - [AD] - data[32]
//  5 - [E] - data[7]        31 - [AE] - data[33]
//  6 - [F] - data[8]        32 - [AF] - data[34]
//  7 - [G] - data[9]        33 - [AG] - data[35]
//  8 - [H] - data[10]       34 - [AH] - data[36]
//  9 - [I] - data[11]       35 - [AI] - data[37]
// 10 - [J] - data[12]       36 - [AJ] - data[38]
// 11 - [K] - data[13]       37 - [AK] - data[39]
// 12 - [L] - data[14]       38 - [AL] - data[40]
// 13 - [M] - data[15]       39 - [AM] - data[41]
// 14 - [N] - data[16]       40 - [AN] - data[42]
// 15 - [O] - data[17]       41 - [AO] - data[43]
// 16 - [P] - data[18]       42 - [AP] - data[44]
// 17 - [Q] - data[19]       43 - [AQ] - data[45]
// 18 - [R] - data[20]       44 - [AR] - data[46]
// 19 - [S] - data[21]       45 - [AS] - data[47]
// 20 - [T] - data[22]       46 - [AT] - data[48]
// 21 - [U] - data[23]       47 - [AU] - data[49]
// 22 - [V] - data[24]       48 - [AV] - data[50]
// 23 - [W] - data[25]       49 - [AW] - data[51]
// 24 - [X] - data[26]       50 - [AX] - data[52]
// 25 - [Y] - data[27]       51 - [AY] - data[53]
// 26 - [Z] - data[28]       52 - [AZ] - data[54]

/*
	bcomp_proc()

	Вызывается из OBD.C
	Парсит PID-s ответы, заносит параметры в структуру bcomp.
 */
void bcomp_proc(int pid, uint8_t *data, uint8_t size) {
	data = data-1;
	/*  Details from: http://en.wikipedia.org/wiki/OBD-II_PIDs */
	switch (pid) {
	//   A       B       C       D       E
	// data[3] data[4] data[5] data[6] data[7]
	case ENGINE_COOLANT_TEMP:
		// A-40 [degree C]
		bcomp.t_engine = data[3] - 40;
		DBG("Engine temperature = %d°C\r\n", (int)bcomp.t_engine);
		break;
	case ENGINE_RPM:
		// ((A*256)+B)/4 [RPM]
		bcomp.rpms = bcomp.rpm = (uint32_t)((data[3]*256) + data[4])/4;
		DBG("Engine RPM = %drpm\r\n", (int)bcomp.rpm);
		break;
	case INTAKE_PRESSURE:                   
		// A [kPa]
		bcomp.p_intake = data[3];
		DBG("Intake Pressure = %dkPa\r\n", (int)bcomp.p_intake);
#if defined( WIN32 )
		diagram_add(&bcomp.dia_intake, (float)bcomp.p_intake);
#endif
		break;
	case FUEL_RAIL_PRES_ALT:
		// ((A*256)+B)*10 [MPa]
		bcomp.p_fuel = ((data[3] * 256)+data[4])*10;
		DBG("Fuel pressure = %dkPa\r\n", (int)bcomp.p_fuel);
		// Давление топлива попадает на график в реальном времени:
		diagram_add(&bcomp.dia_rail, (float)bcomp.p_fuel);
		break;
	case VEHICLE_SPEED:
		// A [km]
		bcomp.speed = data[3];
		DBG("Speed = %dkm\r\n", (int)bcomp.speed);
		break;
	case ECU_VOLTAGE:
		// ((A*256)+B)/1000 [V]
		bcomp.v_ecu = (float)((data[3]*256)+data[4])/1000.0f;
		DBG("Volt = %d.%dV\r\n", (int)bcomp.v_ecu, (int)(bcomp.v_ecu*10.0f)%10);
#if defined( WIN32 )
		diagram_add(&bcomp.dia_voltage, (float)bcomp.v_ecu);
#endif
		break;
#if ( PAJERO_SPECIFIC == 1 )
	case PAJERO_AT_INFO:
		// F-40 [degrees C]
		bcomp.t_akpp = (data[8]-40);
		DBG("AT temperature = %d°C\r\n", (int)bcomp.t_akpp);
#if defined( WIN32 )
		diagram_add(&bcomp.dia_trans, (float)bcomp.t_akpp);
#endif
		break;
#endif
#if ( NISSAN_SPECIFIC == 1 )
	case NISSAN_AT_INFO:
		// This formula analog of Taylor series:
		// (0.000000002344*(AD^5))+(-0.000001387*(AD^4))+(0.0003193*(AD^3))+(-0.03501*(AD^2))+(2.302*AD)+(-36.6) [degrees C]
		// or:
		// (0.01879280128 * AD)^5-(0.03431777443*AD)^4+(0.06834912716*AD)^3-(0.1871095936*AD)^2+(2.302*AD)-36.6  [degrees C]
		// data[32] = AD
		#define AD ((double)data[32])
		bcomp.t_akpp = ((((0.000000002344f * AD - 0.000001387f) * AD + 0.0003193f) * AD - 0.03501f) * AD + 2.302f) * AD - 36.6f;
		#undef AD
		break;
#endif		
	case GET_VIN:
		// VIN-код получен, не требуется больше его читать!
		obd_act_set(GET_VIN, 0);
		// Сохраняем VIN:
		//strcpy(bcomp.vin, (char*)&data[4]);
		memcpy(bcomp.vin, (char*)&data[4], 17);
		bcomp.vin[17] = 0;
		DBG("VIN: %s\r\n", bcomp.vin);
		break;
	case STATUS_DTC:
		if (data[3] & 0x80) {
			// MIL Light on
			//bcomp.mil = 1;
			// FIX: флаг ставится только после чтения кода ошибки
			obd_act_set(FREEZE_DTC, 1);
			DBG("MIL ON (DTCs = %d)!\r\n", data[3]&0x7F);
		} else {
			// MIL Light off
			bcomp.mil = 0;
			DBG("MIL OFF (DTCs = %d)!\r\n", data[3]&0x7F);
		}
		break;
	case FREEZE_DTC: {
#if defined( _DBGOUT )
		char error_code[8];
#endif
		// Деактивируем чтение кода:
		obd_act_set(FREEZE_DTC, 0);
		// Сохраняем код:
		bcomp.e_code = data[3] * 256 + data[4];
		bcomp.mil = 1;
#if defined( _DBGOUT )
		error_decrypt(bcomp.e_code, error_code);
		DBG("Trouble code detect: %s\r\n", error_code);
#endif
		break;
	}
#if ( PAJERO_SPECIFIC == 1 )
	case PAJERO_ODO_INFO:
		// Деактивируем чтение одометра:
		obd_act_set(PAJERO_ODO_INFO, 0);
		// Сохраняем прочитанное значение:
		bcomp.odometer = (data[4] * 256 + data[5]) * 256 + data[6];
		DBG("Odometer in ECU: %dkm", bcomp.odometer);
		break;
#endif
	default:
		break;
	}
}

/*
	bcomp_raw()

	Вызывается из OBD.C. Обработка сырых данных шины. 
	Извлекает инфомрацию из пакетов данных, гуляющих на шине.
	Данные обработки специфичны для Pajero Sport 2nd generation.
 */
void bcomp_raw(int pid, uint8_t *data, uint8_t size) {
	bcomp.connect = 1;
	switch (pid) {
#if ( PAJERO_SPECIFIC == 1 )
	case 0x0215:
		bcomp.speed = ((uint32_t)data[0] * 256 + data[1]) / 128;
		break;
	case 0x0218:
		// AT-коробка в наличии!
		bcomp.at_present = 1;
		// Отображение передачи:
		bcomp.at_drive = (uint8_t)data[2] & 0x0F;
		break;
	case 0x0236:
	case 236:
		// Посылка с датчика положения руля, 
		// пока неизвестно где какие данные:
		memcpy(bcomp.esc_data, data, 8);
		bcomp.esc_id = pid;
		break;
	case 0x0308:
		bcomp.rpms = bcomp.rpm = (uint32_t)data[1] * 256 + data[2];
		if (bcomp.rpm > 500) {
			// Активируем опрос по CAN-шине:
			obd_act(1);
		} else 
		if (bcomp.rpm == 0) {
			// Останавливаем опрос по CAN-шине:
			obd_act(0);
		}
		// NOTE:
		// Эта активация сделана в попытках борьбы с ошибкой P1901.
		// На аппаратной версии LPC17xx ошибка не проявляется, 
		// на аппаратной версии LPC11Cxx ошибка проявляется.
		// В настоящий момент причина до конца не понятна.
		break;
	case 0x0608:
		bcomp.t_engine = (int32_t)data[0] - 40;
		// ТЕСТОВЫЙ ВАРИАНТ ПОТРЕБЛЕНИЯ ТОПЛИВА
		bcomp.raw_fuel = (int32_t)data[5]*256 + data[6];
#if defined( WIN32 )
		diagram_add(&bcomp.dia_engine, (float)bcomp.t_engine);
#endif
		break;
#endif
#if ( NISSAN_SPECIFIC == 1 )
#endif
#if 0
	// OTHER VENDOR SPECIFIC CAN PAKCETS PROCESS
#endif
	default:
		break;
	}	
}

/*
	bcomp_calc()

	Функция рассчета данных. Вызывается каждую секунду.
	Проводит интегрирование показаний и пересчет различных параметров маршрута.
 */
void bcomp_calc(void) {
	int i;
	double d_dist = (double)bcomp.speed / 3600.0f * bconfig.speed_coeff * 1000.0f;
	double d_fuel = (double)bcomp.raw_fuel / 3600.0f * bconfig.fuel_coeff / 1000.0f;

	DBG("bcomp_calc()\r\n");

	// Обновляем съеденное топливо:
	bcomp.fuel += d_fuel;
	bcomp.dist += d_dist;

	// Обновление маршрутов:
	if (bcomp.rpm) {
		for (i=0; i<2; i++) {
			bcomp.trip[i].dist += d_dist;
			bcomp.trip[i].fuel += d_fuel;
			bcomp.trip[i].time++;
		}
	}

	// Сервисное время (только если двигатель запущен):
	if (bcomp.rpm) {
		// Счетчик секунд:
		bcomp.time++;
		// Счетчики сервисные:
		bcomp.moto_time++;
		bcomp.moto_time_service++;
		// Дистанции:
		bcomp.moto_dist += d_dist;
		bcomp.moto_dist_service += d_dist;
	}

	if ((bcomp.time % 30) == 0) {
		// Таблица для рассчета потребления топлива:
		bcomp.log[(bcomp.time/30)%20].fuel = bcomp.fuel;
		bcomp.log[(bcomp.time/30)%20].dist = bcomp.dist;
	}
	
	if ((bcomp.time % 60) == 0) {
		// Каждую минуту заносим:
		diagram_add(&bcomp.dia_engine,  (float)bcomp.t_engine);
		diagram_add(&bcomp.dia_trans,   (float)bcomp.t_akpp);
		//diagram_add(&bcomp.dia_rail,    (float)bcomp.p_fuel);
		diagram_add(&bcomp.dia_intake,  (float)bcomp.p_intake);
		diagram_add(&bcomp.dia_voltage, (float)bcomp.v_ecu);
	}

	// Обнуляем параметры:
	bcomp.rpm = 0;

	if (bcomp.utime) {
		if (bcomp.g_correct) {
			// nop
		} else {
			bcomp.utime++;
		}
	}

	event_set(bcomp_calc, 1000);
}

/*
	bcomp_analog()
	
	Чтение аналоговых значний (АЦП-входы).
 */
#define ABS(a) ((a)>0?(a):-(a))
#define MAX_FUEL_DIFF 3.0f
void bcomp_analog(void) {
	static int fuel_protect_cnt = 0;
	float new_fuel_level;
	DBG("bcomp_analog();\r\n");
	// Внешний датчик температуры:
	bcomp.t_ext = analog_temp(&bconfig.termistor);
	// Данные о напряжении:
	bcomp.v_analog = analog_volt(); 
	// Данные с ДУТ:
	new_fuel_level = analog_fuel();
	if (ABS(bcomp.fuel_level - new_fuel_level) > MAX_FUEL_DIFF) {
		fuel_protect_cnt++;
		if (fuel_protect_cnt > 6) {
			// Если фиксируется больше 18 секунд, тогда сбрасываем путь.
			// Защищает от случайного сброса, если вдруг пройдет какая-то помеха
			// на аналоговой линии.
			DBG("Fueling detect! Reset Trip B, previous fuel count: %d!\r\n", (int)bcomp.trip[1].fuel);

			// Сбрасываем путь "Б":
			bcomp.trip[1].dist = 0.0f;
			bcomp.trip[1].time = 0;
			bcomp.trip[1].fuel = 0;
			// Требуется сохранить изменения в EEPROM, ставим флаг сохранения:
			save_flag |= 0x01;
		}
	} else {
		fuel_protect_cnt = 0;
	}
	bcomp.fuel_level = new_fuel_level;
	event_set(bcomp_analog, 3000);
}

/*
	bcomp_save()

	Асинхронное событие сохранения данных: 
 */
void bcomp_save(void) {
	save_flag |= 0x01;
	event_set(bcomp_save, 30000);
}

/*
	bcomp_elog()

	Асинхронное событие сохранения лога: 
 */
void bcomp_elog(void) {
	save_flag |= 0x08;
	event_set(bcomp_elog, 1000);
}

// -----------------------------------------------------------------------------
// save_XXX
// Функции сохранения различных параметров
// -----------------------------------------------------------------------------

void save_params(void) {
	DBG("save_params()\r\n");
	// Сервисные пробеги и моточасы:
	config_save(CPAR_MOTO_GLOB, (uint8_t*)&bcomp.moto_time, CPAR_MOTO_GLOB_SIZE);
	config_save(CPAR_MOTO_SERV, (uint8_t*)&bcomp.moto_time_service, CPAR_MOTO_SERV_SIZE);
	config_save(CPAR_DIST_GLOB, (uint8_t*)&bcomp.moto_dist, CPAR_DIST_GLOB_SIZE);
	config_save(CPAR_DIST_SERV, (uint8_t*)&bcomp.moto_dist_service, CPAR_DIST_SERV_SIZE);
	// Инициализация параметров маршрута А:
	config_save(CPAR_TRIPA_DIST, (uint8_t*)&bcomp.trip[0].dist, CPAR_TRIPA_DIST_SIZE);
	config_save(CPAR_TRIPA_TIME, (uint8_t*)&bcomp.trip[0].time, CPAR_TRIPA_TIME_SIZE);
	config_save(CPAR_TRIPA_FUEL, (uint8_t*)&bcomp.trip[0].fuel, CPAR_TRIPA_FUEL_SIZE);
	// Инициализация параметров маршрута Б:
	config_save(CPAR_TRIPB_DIST, (uint8_t*)&bcomp.trip[1].dist, CPAR_TRIPB_DIST_SIZE);
	config_save(CPAR_TRIPB_TIME, (uint8_t*)&bcomp.trip[1].time, CPAR_TRIPB_TIME_SIZE);
	config_save(CPAR_TRIPB_FUEL, (uint8_t*)&bcomp.trip[1].fuel, CPAR_TRIPB_FUEL_SIZE);
	// Прочее:
	config_save(CPAR_FUEL_LEVEL, (uint8_t*)&bcomp.fuel_level, CPAR_FUEL_LEVEL_SIZE);
}

void save_settings(void) {
	DBG("save_settings()\r\n");
	// Сохраняем настройки:
	config_save(CPAR_SETUP_V_MAX, (uint8_t*)&bcomp.setup.v_max, CPAR_SETUP_V_MAX_SIZE);
	config_save(CPAR_SETUP_V_MIN, (uint8_t*)&bcomp.setup.v_min, CPAR_SETUP_V_MIN_SIZE);
	config_save(CPAR_SETUP_T_AT, (uint8_t*)&bcomp.setup.t_at, CPAR_SETUP_T_AT_SIZE);
	config_save(CPAR_SETUP_T_ENG, (uint8_t*)&bcomp.setup.t_eng, CPAR_SETUP_T_ENG_SIZE);
	config_save(CPAR_SETUP_F_FUEL, (uint8_t*)&bcomp.setup.f_fuel, CPAR_SETUP_F_FUEL_SIZE);
	config_save(CPAR_SETUP_L_FUEL, (uint8_t*)&bcomp.setup.l_fuel, CPAR_SETUP_L_FUEL_SIZE);
	//config_save(CPAR_SETUP_TIME, (uint8_t*)&bcomp.setup.time, CPAR_SETUP_TIME_SIZE);
	config_save(CPAR_SETUP_W_DELAY, (uint8_t*)&bcomp.setup.w_delay, CPAR_SETUP_W_DELAY_SIZE);
	config_save(CPAR_SETUP_F_EXT, (uint8_t*)&bcomp.setup.f_ext, CPAR_SETUP_F_EXT_SIZE);
	config_save(CPAR_SETUP_F_EXT_W, (uint8_t*)&bcomp.setup.f_ext_w, CPAR_SETUP_F_EXT_W_SIZE);
	config_save(CPAR_SETUP_T_EXT, (uint8_t*)&bcomp.setup.t_ext, CPAR_SETUP_T_EXT_SIZE);
	config_save(CPAR_SETUP_F_GPS, (uint8_t*)&bcomp.setup.f_gps, CPAR_SETUP_F_GPS_SIZE);
	config_save(CPAR_SETUP_I_GPS, (uint8_t*)&bcomp.setup.i_gps, CPAR_SETUP_I_GPS_SIZE);
	config_save(CPAR_SETUP_F_ESP, (uint8_t*)&bcomp.setup.f_esp, CPAR_SETUP_F_ESP_SIZE);
	config_save(CPAR_SETUP_FUEL_CAL, (uint8_t*)&bcomp.setup.fuel_cal, CPAR_SETUP_FUEL_CAL_SIZE);
	config_save(CPAR_SETUP_F_LOG, (uint8_t*)&bcomp.setup.f_log, CPAR_SETUP_F_LOG_SIZE);
	config_save(CPAR_SETUP_F_CONTRAST, (uint8_t*)&bcomp.setup.contrast, CPAR_SETUP_F_CONTRAST_SIZE);
	config_save(CPAR_SETUP_F_SOUND, (uint8_t*)&bcomp.setup.sound, CPAR_SETUP_F_SOUND_SIZE);
}

#if !defined( WIN32 )
void ProtectDelay(void) {
	int n;
	for (n = 0; n < 100000; n++) { __NOP(); } 
}
#endif

#if defined( WIN32 )
// Win-версия запускает основную логику в дополнительном потоке:
DWORD WINAPI ProcMain(LPVOID par)
#else
int main(void)
#endif
{
	int ret;
	int ms;
	char str[20];

	// На всякий случай деинициализируем CAN и 
	// переводим в состояние ожидания (в функции:
	//obd_deinit();

	// Инициализация периферийных модулей и библиотек:
	leds_init();
	led_red(1);
	uart0_init(bconfig.uart_speed);
	event_init();
	button_init();
#if !defined( WIN32 )
	beep_init();
#endif
#if defined( WIN32 )
	ee_init();
#else
	i2c_init();
#endif
	adc_init();
#if ( GRAPH_SUPPORT == 1)
	warning_init();
#endif
#if ( NMEA_SUPPORT == 1 )
	nmea_init();
#endif
	DBG("init ok!\r\n");

	// -----------------------------------------------------------------------------
	// Инициализация переменных:
	// -----------------------------------------------------------------------------
	bcomp.time = 0;
	bcomp.t_engine = 0xFFFF;
	bcomp.rpm = 0;
	bcomp.speed = 0;
#if ( NISSAN_SPECIFIC == 1 )
	bcomp.at_present = 1;
#else
	bcomp.at_present = 0;
#endif
	bcomp.at_drive = 0xFF;
	bcomp.connect = 0;
	bcomp.v_ecu = NAN;
	bcomp.t_akpp = 0xFFFF;
	bcomp.t_ext = 0xFFFF;
	bcomp.dist = 0.0f;
	bcomp.fuel = 0.0f;
	memset(bcomp.vin, 0, sizeof(bcomp.vin));
	bcomp.odometer = -1;
	bcomp.angle = 0;
	bcomp.esc_id = 0;
	bcomp.utime = 0;
	bcomp.nmea_cnt = 0;
	bcomp.g_correct = 0;

#if defined( WIN32 )
	// -----------------------------------------------------------------------------
	// Инициализация для симулятора интерфейса:
	// -----------------------------------------------------------------------------
	bcomp.at_present = 1;
	bcomp.at_drive = 0x02;
#endif

#if defined( WIN32 )
	// Тестовая обработка GPS-данных:
	//nmea_parce("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A");    // year < 2000
	//nmea_parce("$GPRMC,113650.0,A,5548.607,N,03739.387,E,000.01,25 5.6,210403,08.7,E*69"); // bad crc
	nmea_parce("$GPRMC,194530.000,A,3051.8007,N,10035.9989,W,1.49,111.67,310714,,,A*74");
#endif

	// -----------------------------------------------------------------------------
	// Загрузка сохраненных значений и настроек:
	// -----------------------------------------------------------------------------

	// Актуальный экран для отображения:
	if (config_read(CPAR_PAGE, (uint8_t*)&bcomp.page, CPAR_PAGE_SIZE)) {
		bcomp.page = 0;
	}
	// Сервисные пробеги и моточасы:
	if (config_read(CPAR_MOTO_GLOB, (uint8_t*)&bcomp.moto_time, CPAR_MOTO_GLOB_SIZE)) {
		bcomp.moto_time = 0;
	}
	if (config_read(CPAR_MOTO_SERV, (uint8_t*)&bcomp.moto_time_service, CPAR_MOTO_SERV_SIZE)) {
		bcomp.moto_time_service = 0;
	}
	if (config_read(CPAR_DATE_SERV, (uint8_t*)bcomp.moto_date_service, CPAR_DATE_SERV_SIZE)) {
		memset(bcomp.moto_date_service, 0, sizeof(bcomp.moto_date_service));
	}
	// Глобальные пробеги и моточасы:
	if (config_read(CPAR_DIST_GLOB, (uint8_t*)&bcomp.moto_dist, CPAR_DIST_GLOB_SIZE)) {
		bcomp.moto_dist = 0.0f;
	}
	if (config_read(CPAR_DIST_SERV, (uint8_t*)&bcomp.moto_dist_service, CPAR_DIST_SERV_SIZE)) {
		bcomp.moto_dist_service = 0.0f;
	}
	// Инициализация параметров маршрута А:
	if (config_read(CPAR_TRIPA_DIST, (uint8_t*)&bcomp.trip[0].dist, CPAR_TRIPA_DIST_SIZE)) {
		bcomp.trip[0].dist = 0.0f;
	}
	if (config_read(CPAR_TRIPA_TIME, (uint8_t*)&bcomp.trip[0].time, CPAR_TRIPA_TIME_SIZE)) {
		bcomp.trip[0].time = 0;
	}
	if (config_read(CPAR_TRIPA_FUEL, (uint8_t*)&bcomp.trip[0].fuel, CPAR_TRIPA_FUEL_SIZE)) {
		bcomp.trip[0].fuel = 0.0f;
	}
	// Инициализация параметров маршрута Б:
	if (config_read(CPAR_TRIPB_DIST, (uint8_t*)&bcomp.trip[1].dist, CPAR_TRIPB_DIST_SIZE)) {
		bcomp.trip[1].dist = 0.0f;
	}
	if (config_read(CPAR_TRIPB_TIME, (uint8_t*)&bcomp.trip[1].time, CPAR_TRIPB_TIME_SIZE)) {
		bcomp.trip[1].time = 0;
	}
	if (config_read(CPAR_TRIPB_FUEL, (uint8_t*)&bcomp.trip[1].fuel, CPAR_TRIPB_FUEL_SIZE)) {
		bcomp.trip[1].fuel = 0.0f;
	}
	// Переменная одометра:
	//if (config_read(CPAR_SETUP_ODO, (uint8_t*)&bcomp.odometer, CPAR_SETUP_ODO_SIZE)) {
	//	bcomp.odometer = -1;
	//}
	// Инициализация настроечных переменных:
	if (config_read(CPAR_SETUP_V_MAX, (uint8_t*)&bcomp.setup.v_max, CPAR_SETUP_V_MAX_SIZE)) {
		bcomp.setup.v_max = bconfig.v_max;
	}
	if (config_read(CPAR_SETUP_V_MIN, (uint8_t*)&bcomp.setup.v_min, CPAR_SETUP_V_MIN_SIZE)) {
		bcomp.setup.v_min = bconfig.v_min;
	}
	// Температура предупреждения о перегреве автоматической коробки:
	if (config_read(CPAR_SETUP_T_AT, (uint8_t*)&bcomp.setup.t_at, CPAR_SETUP_T_AT_SIZE)) {
		bcomp.setup.t_at = bconfig.t_akpp_warning;
	}
	// Температура предупреждения о перегреве двигателя:
	if (config_read(CPAR_SETUP_T_ENG, (uint8_t*)&bcomp.setup.t_eng, CPAR_SETUP_T_ENG_SIZE)) {
		bcomp.setup.t_eng = bconfig.t_engine_warning;
	}
	// Флаг подведенного сигнала уровня топлива в баке:
	if (config_read(CPAR_SETUP_F_FUEL, (uint8_t*)&bcomp.setup.f_fuel, CPAR_SETUP_F_FUEL_SIZE)) {
		bcomp.setup.f_fuel = 0;
	}
	// Уровень топлива предупреждения на пустой бак:
	if (config_read(CPAR_SETUP_L_FUEL, (uint8_t*)&bcomp.setup.l_fuel, CPAR_SETUP_L_FUEL_SIZE)) {
		bcomp.setup.l_fuel = 10.0f;
	}
	// Смещение часового пояса:
	//if (config_read(CPAR_SETUP_TIME, (uint8_t*)&bcomp.setup.time, CPAR_SETUP_TIME_SIZE)) {
	//	bcomp.setup.time = 3600*3;
	//}
	// Задержка повтора предупреждений:
	if (config_read(CPAR_SETUP_W_DELAY, (uint8_t*)&bcomp.setup.w_delay, CPAR_SETUP_W_DELAY_SIZE)) {
		bcomp.setup.w_delay = 30;
	}
	// Флаг наличия внешнего датчика:
	if (config_read(CPAR_SETUP_F_EXT, (uint8_t*)&bcomp.setup.f_ext, CPAR_SETUP_F_EXT_SIZE)) {
		bcomp.setup.f_ext = 1;
	}
	// Флаг вывода предупреждения о гололёде:
	if (config_read(CPAR_SETUP_F_EXT_W, (uint8_t*)&bcomp.setup.f_ext_w, CPAR_SETUP_F_EXT_W_SIZE)) {
		bcomp.setup.f_ext_w = 0;
	}
	// Температура срабатывания предупреждения о гололёде:
	if (config_read(CPAR_SETUP_T_EXT, (uint8_t*)&bcomp.setup.t_ext, CPAR_SETUP_T_EXT_SIZE)) {
		bcomp.setup.t_ext = 1;
	}
	// Флаг наличия GPS-приемника:
	if (config_read(CPAR_SETUP_F_GPS, (uint8_t*)&bcomp.setup.f_gps, CPAR_SETUP_F_GPS_SIZE)) {
		bcomp.setup.f_gps = 0;
	}
	// Скорость работы UART-интерфейса:
	if (config_read(CPAR_SETUP_I_GPS, (uint8_t*)&bcomp.setup.i_gps, CPAR_SETUP_I_GPS_SIZE)) {
		bcomp.setup.i_gps = bconfig.uart_speed;
	} else {
		DBG("UART speed = %d\r\n", bcomp.setup.i_gps);
		// Если скорости отличается от настроечной, перенастраиваем UART:
		if (bcomp.setup.i_gps != bconfig.uart_speed) {
			uart0_init(bcomp.setup.i_gps);
		}
	}
	// Флаг наличия системы стабилизации, для неё имеется CAN-датчик положения рулевого колеса:
	if (config_read(CPAR_SETUP_F_ESP, (uint8_t*)&bcomp.setup.f_esp, CPAR_SETUP_F_ESP_SIZE)) {
		bcomp.setup.f_esp = 0;
	}
#if ( ELOG_SUPPORT == 1 )
	// Флаг отправки лог-данных в UART-порт. Можно подключать устройство логгирования UART-данных.
	// Данные отправляются на той же скорости, что работает GPS-модуль.
	if (config_read(CPAR_SETUP_F_LOG, (uint8_t*)&bcomp.setup.f_log, CPAR_SETUP_F_LOG_SIZE)) {
		bcomp.setup.f_log = bconfig.elog_flag;
	}
#endif
	// Калибровочный коэффициент топлива:
	// Редактировать его требуется по формуле: F_новый_коэффициент = L_заправки / L_израсходованного * F_старый_коэффициент.
	if (config_read(CPAR_SETUP_FUEL_CAL, (uint8_t*)&bcomp.setup.fuel_cal, CPAR_SETUP_FUEL_CAL_SIZE)) {
		bcomp.setup.fuel_cal = bconfig.fuel_coeff;
	}
	// Значение яркости экрана:
	if (config_read(CPAR_SETUP_F_CONTRAST, (uint8_t*)&bcomp.setup.contrast, CPAR_SETUP_F_CONTRAST_SIZE)) {
		bcomp.setup.contrast = bconfig.contrast;
	}
	// Флаг, проигрывать ли звуки:
	if (config_read(CPAR_SETUP_F_SOUND, (uint8_t*)&bcomp.setup.sound, CPAR_SETUP_F_SOUND_SIZE)) {
		bcomp.setup.sound = 0;
	}
	// Флаг сервисного режима, сохраняется между сеансами работы, активируется на время сервисного обслуживания:
	if (config_read(CPAR_SERVICE, (uint8_t*)&bcomp.service, CPAR_SERVICE_SIZE)) {
		bcomp.service = 0;
	}
	// Текущий уровень топлива:
	if (config_read(CPAR_FUEL_LEVEL, (uint8_t*)&bcomp.fuel_level, CPAR_FUEL_LEVEL_SIZE)) {
		bcomp.fuel_level = 0.0f;
	}
	
	// -----------------------------------------------------------------------------
	// Инициализация диаграмм:
	// -----------------------------------------------------------------------------
	
	diagram_create(&bcomp.dia_engine, 100, 50, 1); // от 50 до 100 градусов.
	diagram_create(&bcomp.dia_trans, 115, 50, 1);  // от 50 до 115 градусов.
	diagram_create(&bcomp.dia_rail, 60000, 10000, 1);   // от 10 до 60 МПа (от 100 до 600Атм).
	diagram_create(&bcomp.dia_intake, 200, 80, 1); // от 0.8 до 2 Атм.
	diagram_create(&bcomp.dia_voltage, 15.0f, 11.0f, 1); // от 11V до 15V Атм.
	
	// -----------------------------------------------------------------------------
	// Инициализируем асинхронные события:
	// -----------------------------------------------------------------------------

	// Процедура 
	event_set(bcomp_calc, 1000); delay_ms(10);
	// Получение аналоговых данных:
	event_set(bcomp_analog, 500); delay_ms(10);
#if ( ELOG_SUPPORT == 1 )
	// Если установлен флаг логгирования, инициализируем соотв. событие:
	if (bcomp.setup.f_log) {
		event_set(bcomp_elog, 10000); delay_ms(10);
	}
#endif
#if ( WARNING_SUPPORT == 1 )
	// Асинхронный обработчик с проверками на предупреждения:
	event_set(bcomp_warning, 10000); delay_ms(10);
#endif
	// Сохраняем данные в EEPROM, только если задана её конфигурация:
	event_set(bcomp_save, 30000); delay_ms(10);

	// -----------------------------------------------------------------------------
	// Инициализируем экран:
	// -----------------------------------------------------------------------------

#if ( GRAPH_SUPPORT == 1 )
	oled_init(bcomp.setup.contrast, 0);
	graph_clear();
#endif

	// -----------------------------------------------------------------------------
	// Запускаем OBD-протокол:
	// -----------------------------------------------------------------------------
	obd_init();

	// -----------------------------------------------------------------------------
	// Стартовый экран (заставка/мелодия):
	// -----------------------------------------------------------------------------

#if ( GRAPH_SUPPORT == 1 )
	if (bconfig.start_delay) {
#if ( PAJERO_SPECIFIC == 1 )
		// Новая версия заставки, только иконка:
		graph_pic(&ico64_mitsu,64-32,0);
#elif ( NISSAN_SPECIFIC == 1 )
		// Новая версия заставки, только иконка:
		graph_pic(&ico64_nissan,64-36,8);
#else
		// Просто картинка заставки:
		graph_pic(&ico48_mcu,64-24,8);
#endif
		graph_update();
	}
#endif
	if (bconfig.start_sound) {
#if !defined( WIN32 )
		beep_play(melody_start);
#endif
	}
	if (bconfig.start_delay) {
		// Ожидание N секунд, по нажатию - прерываем.
		ms = get_ms_timer();
		while ((get_ms_timer() - ms) < bconfig.start_delay*1000) {
			__WFI();
			ret = button_read();
			if (ret) {
				break;
			}
		}
	} else {
		// Задержка перед основной логикой:
		delay_ms(1000);
	}

	// Инцициализация закончена, выключаем красный светодиод:
	led_red(0);
		
	// -----------------------------------------------------------------------------
	// Основной цикл работы интерфейса:
	// -----------------------------------------------------------------------------
	ret = 0;
	while (1) {
		int buttons;
#if ( GRAPH_SUPPORT == 1 )
		graph_clear();
#endif
		// Состояние кнопок:
		ms = get_ms_timer();
		while ((get_ms_timer() - ms) < 400) {
			__WFI();
			buttons = button_read();
			if (buttons) {
				break;
			}
		}
		// Вывод страницы:
		DBG("bcomp.page = %08x\r\n", bcomp.page);
		// Обработка кнопок:
		if (buttons & BUTT_SW1) {
			DBG("buttons(): BUTT_SW1\r\n");
			bcomp.page &= ~GUI_FLAG_GRAPH;
#if !defined( WIN32 )
			if (bcomp.setup.sound) {
				// Нажатие на кнопку:
				beep_play(melody_wrep);
			}
#endif
			if (bcomp.page & GUI_FLAG_MENU) {
				// nop
			} else
#if ( WARNING_SUPPORT == 1 )
			if (bcomp.page & GUI_FLAG_WARNING) {
				// nop
			} else 
#endif
			{
				if (bcomp.page == 8) {
					if (bcomp.service & 0x80) {
						bcomp.service ^= 0x01;
						config_save(CPAR_SERVICE, (uint8_t*)&bcomp.service, CPAR_SERVICE_SIZE);
						goto end_sw1_proc;
					}
				}
				bcomp.page++;
				config_save(CPAR_PAGE, (uint8_t*)&bcomp.page, CPAR_PAGE_SIZE);
			}
		}
end_sw1_proc:
		if (buttons & BUTT_SW1_LONG) {
			DBG("buttons(): BUTT_SW1_LONG\r\n");
#if !defined( WIN32 )
			if (bcomp.setup.sound) {
				// Нажатие на кнопку:
				beep_play(melody_wrep2);
			}
#endif
			if (bcomp.page & GUI_FLAG_MENU) {
				// nop
			} else 
#if ( WARNING_SUPPORT == 1 )
			if (bcomp.page & GUI_FLAG_WARNING) {
				// nop
			} else 
#endif
			if (bcomp.page == 1) {
				buttons = 0;
				bcomp.page |= GUI_FLAG_MENU;
			} else 
			if (bcomp.page == 2) {
				bcomp.page ^= GUI_FLAG_GRAPH;
			} else 
			if (bcomp.page == 3) {
				bcomp.page ^= GUI_FLAG_GRAPH;
			} else 
			if (bcomp.page == 4) {
				bcomp.page ^= GUI_FLAG_GRAPH;
			} else 
			if (bcomp.page == 6) {
				bcomp.trip[0].dist = 0;
				bcomp.trip[0].time = 0;
				bcomp.trip[0].fuel = 0;
				// Сброс параметров маршрута А:
				config_save(CPAR_TRIPA_DIST, (uint8_t*)&bcomp.trip[0].dist, CPAR_TRIPA_DIST_SIZE);
				config_save(CPAR_TRIPA_TIME, (uint8_t*)&bcomp.trip[0].time, CPAR_TRIPA_TIME_SIZE);
				config_save(CPAR_TRIPA_FUEL, (uint8_t*)&bcomp.trip[0].fuel, CPAR_TRIPA_FUEL_SIZE);
			} else 
			if (bcomp.page == 7) {
				bcomp.trip[1].dist = 0;
				bcomp.trip[1].time = 0;
				bcomp.trip[1].fuel = 0;
				// Сброс параметров маршрута Б:
				config_save(CPAR_TRIPB_DIST, (uint8_t*)&bcomp.trip[1].dist, CPAR_TRIPB_DIST_SIZE);
				config_save(CPAR_TRIPB_TIME, (uint8_t*)&bcomp.trip[1].time, CPAR_TRIPB_TIME_SIZE);
				config_save(CPAR_TRIPB_FUEL, (uint8_t*)&bcomp.trip[1].fuel, CPAR_TRIPB_FUEL_SIZE);
			} else 
			if (bcomp.page == 8) {
				if (bcomp.service == 0) {
					bcomp.service = 0x80;
				} else {
					if (bcomp.service & 0x01) {
						// Инициализация сервисного счетчика.
						// Моточасы:
						bcomp.moto_time_service = 0;
						config_save(CPAR_MOTO_SERV, (uint8_t*)&bcomp.moto_time_service, CPAR_MOTO_SERV_SIZE);
						// Дистанция:
						bcomp.moto_dist_service = 0.0f;
						config_save(CPAR_DIST_SERV, (uint8_t*)&bcomp.moto_dist_service, CPAR_DIST_SERV_SIZE);
						// Дата проведения ТО:
						memcpy(bcomp.moto_date_service, bcomp.gps_val_date,sizeof(bcomp.moto_date_service));
						bcomp.moto_date_service[sizeof(bcomp.moto_date_service)-1] = 0;
						config_save(CPAR_DATE_SERV, (uint8_t*)bcomp.moto_date_service, CPAR_DATE_SERV_SIZE);
					}
					bcomp.service = 0;
				}
				config_save(CPAR_SERVICE, (uint8_t*)&bcomp.service, CPAR_SERVICE_SIZE);
			} else
			if (bcomp.page == 11) {
				bcomp.page ^= GUI_FLAG_GRAPH;
			} else 
			if (bcomp.page == 12) {
				bcomp.page ^= GUI_FLAG_GRAPH;
			} 

		}
//end_sw1_long_proc:
		if (buttons & BUTT_SW2) {
			DBG("buttons(): BUTT_SW2\r\n");
			bcomp.page &= ~GUI_FLAG_GRAPH;
#if !defined( WIN32 )
			if (bcomp.setup.sound) {
				// Нажатие на кнопку:
				beep_play(melody_wrep);
			}
#endif
			if (bcomp.page & GUI_FLAG_MENU) {
				// nop
			} else
#if ( WARNING_SUPPORT == 1 )
			if (bcomp.page & GUI_FLAG_WARNING) {
				// nop
			} else 
#endif
			{
				if (bcomp.page == 8) {
					if (bcomp.service & 0x80) {
						bcomp.service ^= 0x01;
						config_save(CPAR_SERVICE, (uint8_t*)&bcomp.service, CPAR_SERVICE_SIZE);
						goto end_sw2_proc;
					}
				}
				bcomp.page--;
				//DBG("config_save(): new page %d\r\n", bcomp.page);
				config_save(CPAR_PAGE, (uint8_t*)&bcomp.page, CPAR_PAGE_SIZE);
			}
		}
end_sw2_proc:
		if (buttons & BUTT_SW2_LONG) {
			DBG("buttons(): BUTT_SW2_LONG\r\n");
		}
repeate:
#if ( WARNING_SUPPORT == 1 )
		// Проверка, есть ли варининги: внутри устанавливается нужный флаг для работы.
		warning_check();
		// Вывод справки по страницам:
		//DBG("page = %d (buttons = %02x)\r\n", bcomp.page, buttons);
		// Проверка флагов, потом основной SWITCH по командам.
		if (bcomp.page & GUI_FLAG_WARNING) {
			ret = warning_show(&buttons);
			if (ret) {
				goto repeate;
			}
		} 
#endif
#if ( GRAPH_SUPPORT == 1 ) && ( WARNING_SUPPORT == 1 )
		else
#endif
#if ( GRAPH_SUPPORT == 1 )
		if (bcomp.page & GUI_FLAG_MENU) {
			int contrast;
			contrast = bcomp.setup.contrast;
			ret = menu_work(&buttons);
			// Проверяем, не изменился ли контраст:
			if (contrast != bcomp.setup.contrast) {
				// Изменился, перенастраиваем:
				oled_contrast(bcomp.setup.contrast);
			}
			switch (ret) {
			case 0x00:
				bcomp.page &= ~GUI_FLAG_MENU;
				goto repeate;
			case 0xF0:
				if (buttons & (BUTT_SW1|BUTT_SW1_LONG)) {
					menu_back();
				}
				graph_puts16(64,  0, 1, INFO_DEVICE);
				graph_puts16(64, 16, 1, INFO_VERSION);
				graph_puts16(64, 32, 1, INFO_AUTHOR);
				graph_puts16(64, 48, 1, INFO_YEAR);
				break;
			case 0xF1:
				DBG("Screen 0xF1, buttons: %02x\r\n", buttons);
				graph_puts16(64, 10, 1, "Сохранение");
				if (buttons & BUTT_SW1) {
					save_flag ^= 0x80;
				}
				if (buttons & BUTT_SW1_LONG) {
					if (save_flag & 0x80) {
						save_flag |= 0x02;
					} else {
						save_flag = 0;
					}
					menu_back();
				}
				if (save_flag & 0x80) {
					graph_puts16(64, 26, 1, "OK");
				} else {
					graph_puts16(64, 26, 1, "Отмена");
				}
				break;
			case 0xF2:
				if (buttons & (BUTT_SW1|BUTT_SW1_LONG)) {
					menu_back();
				}
				graph_puts16(64, 10, 1, "VIN");
				memcpy(str, bcomp.vin, 10); str[10] = 0;
				graph_puts16(64, 26, 1, str);
				memcpy(str, &bcomp.vin[10], 10); str[10] = 0;
				graph_puts16(64, 42, 1, str);
				break;
			case 0xF3:
				if (bcomp.odometer == -1) {
					obd_act_set(PAJERO_ODO_INFO, 1);
				}
				graph_puts16(64, 10, 1, "Пробег ECU");
				_sprintf(str, "%dкм", bcomp.odometer);
				graph_puts16(64, 26, 1, str);
#if 0
				// NOTE:
				// Сохранение пробега ECU для синхронизаций показаний отключено.
				// Не решено окончательно, каким образом точно будет работать данная функция.
				if (buttons & BUTT_SW1) {
					save_flag ^= 0x80;
				}
				if (buttons & BUTT_SW1_LONG) {
					if (save_flag & 0x80) {
						save_flag |= 0x04;
					} else {
						save_flag = 0;
					}
					menu_back();
				}
				if (save_flag & 0x80) {
					graph_puts16(64, 42, 1, "Синхрон.?");
				} else {
					graph_puts16(64, 42, 1, "Отмена");
				}
#endif
				break;
#if 0
			case 0xF4:
				{
					static int i = 0;
					uint8_t data;
					for ( ; i < 2048; i++) {
						if ((i%16) == 0) {
							DBG("\r\n%04x: ", i);
						}
						ee_read(i, &data, 1);
						DBG("%02x", data);
					}
					if (i == 2048) {
						DBG("\r\n");
						i++;
					}
				}
				break;
#endif
			default:
				break;
			}
		} else {
			switch (BCOMP_PAGE()) {
			// SCREENS:
			//  1 - ODOMETER
			//  2 - ENGINE
			//  3 - TRANSMISSION (if present)
			//  4 - BATTERY
			//  5 - FUEL ECONOMY
			//  6 - TRIP A
			//  7 - TRIP B
			//  8 - SERVICE
			//  9 - WHEELS (if present)
			// 10 - GPS (if present)
			// 11 - RAIL PRESSURE
			// 12 - INTAKE PRESSURE
			case 1:
				// -----------------------------------------------------------------
				// ODOMETER
				// -----------------------------------------------------------------
				if (bcomp.setup.f_ext) {
					if (bcomp.t_ext == 0xFFFF) {
						_sprintf(str, "--°C", bcomp.t_ext);
					} else {
						_sprintf(str, "%d°C", bcomp.t_ext);
						graph_puts16(64+32, 0, 1, str);
					}
				}
#if ( PAJERO_SPECIFIC == 1 )
				if (bcomp.at_present) {
					show_drive(64, 14);
				} else
#endif
				{
					int speed;
#if ( NMEA_SUPPORT == 1 )
					if (bcomp.g_correct) {
						speed = (int)bcomp.gps_speed;
					} else 
#endif
					{
						speed = bcomp.speed;
					}
					_sprintf(str, "%dкм/ч", speed);
					graph_puts16(64, 32, 1, str);
				}

				_sprintf(str, "%dкм", (int)bcomp.moto_dist/1000 + bconfig.moto_dist_offset);
				graph_puts16(64, 48, 1, str);
				break;
			case 2:
				// -----------------------------------------------------------------
				// ENGINE
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "ENGINE");
				if (bcomp.t_engine == 0xFFFF) {
					_sprintf(str, "--°C");
				} else {
					_sprintf(str, "%d°C", bcomp.t_engine);
				}
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_engine);
				} else {
					graph_puts32c(64, 24, str);
				}
				break;
			case 3:
				// -----------------------------------------------------------------
				// TRANSMISSION
				// -----------------------------------------------------------------
				if (bcomp.at_present == 0) {
					if (buttons & BUTT_SW2) {
						BCOMP_PAGE_PREV(); //bcomp.page--;
					} else {
						BCOMP_PAGE_NEXT(); //bcomp.page++;
					}
					goto repeate;
				}
#if ( PAJERO_SPECIFIC == 1 ) || ( NISSAN_SPECIFIC == 1 )
				graph_puts16(64, 0, 1, "TRANS");
				if (bcomp.t_akpp == 0xFFFF) {
					_sprintf(str, "--°C");
				} else {
					_sprintf(str, "%d°C", bcomp.t_akpp);
				}
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_trans);
				} else {
#if ( PAJERO_SPECIFIC == 1 )
					show_drive(64, 14);
#endif
					graph_puts32c(64, 38, str);
				}
#endif
				break;
			case 4:
				// -----------------------------------------------------------------
				// BATERY
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "BATTERY");
				if (isnan(bcomp.v_ecu)) {
					_sprintf(str, "--.-V");
				} else {
					_sprintf(str, "%d.%dV", (int)bcomp.v_ecu, (int)(bcomp.v_ecu*10)%10);
				}
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_voltage);
				} else {
					graph_puts32c(64, 24, str);
				}
				break;
			case 5:
				// -----------------------------------------------------------------
				// FUEL ECONOMY
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "FUEL");
				// NOTE: Данный код показывает расход менее чем через 10 минут, это побочное 
				// действие, из-за нулевого значения в ячейке "+1". Однако, это вполне правильный 
				// вариант, постепенно расход подойдет к среднему за 10 минут.
				if ((bcomp.log[(bcomp.time/30)%20].dist - bcomp.log[(bcomp.time/30+1)%20].dist) > 1000.0f) {
					// Если за 10 минут проехали больше 1км:
					float d_fuel = (bcomp.log[(bcomp.time/30)%20].fuel - bcomp.log[(bcomp.time/30+1)%20].fuel);
					float d_dist = (bcomp.log[(bcomp.time/30)%20].dist - bcomp.log[(bcomp.time/30+1)%20].dist)/1000.0f;
					float fuel_km = d_fuel / d_dist * 100.0f;
					if (fuel_km < 50.0f) {
						_sprintf(str, "%2d.%d", (int)fuel_km, (int)(fuel_km*10)%10);
					} else {			  
						_sprintf(str, "--.-");
					}
				} else {			  
					_sprintf(str, "--.-");
				}
				graph_puts32c(64, 14, str);
				graph_ico16(96, 24, ico16_100kms_data, 22); // l/100km
				if (1) { // Часовой расход топлива
					float d_fuel = (bcomp.log[(bcomp.time/30)%20].fuel - bcomp.log[(bcomp.time/30+1)%20].fuel);
					float fuel_h;
					if (bcomp.time < 60) {
						_sprintf(str, "--.-");
					} else
					if (bcomp.time < 600) {
						fuel_h = d_fuel*(3600/((bcomp.time/30)*30));
						_sprintf(str, "%2d.%d", (int)fuel_h, (int)(fuel_h*10)%10);
					} else {
						fuel_h = d_fuel*(3600/600);
						_sprintf(str, "%2d.%d", (int)fuel_h, (int)(fuel_h*10)%10);
					}
				}
				graph_puts32c(64, 38, str);
				graph_ico16(96, 48, ico16_lhour_data, 22); // l/100km
				break;
			case 6:
				// -----------------------------------------------------------------
				// TRIP A
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "TRIP A");
				goto trip;
			case 7:
				// -----------------------------------------------------------------
				// TRIP B
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "TRIP B");
trip:
				_sprintf(str, "%dкм", (int)bcomp.trip[BCOMP_PAGE()-6].dist/1000);
				graph_puts16(64, 16, 1, str);
				_sprintf(str, "%dч%02dм", (int)bcomp.trip[BCOMP_PAGE()-6].time/3600, (int)(bcomp.trip[BCOMP_PAGE()-6].time/60)%60);
				graph_puts16(64, 32, 1, str);
				_sprintf(str, "%dл", (int)bcomp.trip[BCOMP_PAGE()-6].fuel);
				graph_puts16(64, 48, 1, str);
				break;
			case 8:
				// -----------------------------------------------------------------
				// SERVICE
				// -----------------------------------------------------------------
				graph_puts16(64,  0, 1, "SERVICE");
				if (bcomp.service & 0x80) {
					graph_puts16(64, 16, 1, "Режим");
					graph_puts16(64, 32, 1, "активен");
					if (bcomp.service & 0x01) {
						graph_puts16(64, 48, 1, "Сбросить");
					} else {
						graph_puts16(64, 48, 1, "Выход");
					}
				} else {
					_sprintf(str, "%s", bcomp.moto_date_service);
					graph_puts16(64, 16, 1, str);
					_sprintf(str, "%dh", bcomp.moto_time_service/3600);
					graph_puts16(64, 32, 1, str);
					_sprintf(str, "%dkm", (int)bcomp.moto_dist_service/1000);
					graph_puts16(64, 48, 1, str);
				}
				break;
			case 9:
				// -----------------------------------------------------------------
				// WHEELS
				// -----------------------------------------------------------------
#if ( WHELLS_DRAW_SUPPORT == 1 )
				if (bcomp.esc_id == 0 ||
					bcomp.setup.f_esp == 0) {
					if (buttons & BUTT_SW2) {
						BCOMP_PAGE_PREV(); //bcomp.page--;
					} else {
						BCOMP_PAGE_NEXT(); //bcomp.page++;
					}
					goto repeate;
				}
				graph_puts16(64,0,1,"WHEELS");
				graph_line(40+4,40,88-4,40);
				draw_rect(40,40,bcomp.angle);
				draw_rect(88,40,bcomp.angle);
#else
				if (buttons & BUTT_SW2) {
					BCOMP_PAGE_PREV(); //bcomp.page--;
				} else {
					BCOMP_PAGE_NEXT(); //bcomp.page++;
				}
				goto repeate;			
#endif
				break;
			case 10:
#if ( NMEA_SUPPORT == 1 )
				if (bcomp.setup.f_gps == 0) {
					if (buttons & BUTT_SW2) {
						BCOMP_PAGE_PREV(); //bcomp.page--;
					} else {
						BCOMP_PAGE_NEXT(); //bcomp.page++;
					}
					goto repeate;
				}
				graph_puts16(64,0,1,"GPS");
				if (bcomp.g_correct) {
					_sprintf(str,"%s",bcomp.gps_val_time);
					graph_puts16(64,16,1,str);
					_sprintf(str,"%s",bcomp.gps_val_lon); str[10] = 0; // cutting
					graph_puts16(64,32,1,str);
					_sprintf(str,"%s",bcomp.gps_val_lat); str[10] = 0; // cutting
					graph_puts16(64,48,1,str);
				} else {
					graph_puts16(64,32,1,"NO DATA");
				}
#else
				if (buttons & BUTT_SW2) {
					BCOMP_PAGE_PREV(); //bcomp.page--;
				} else {
					BCOMP_PAGE_NEXT(); //bcomp.page++;
				}
				goto repeate;			
#endif
				break;
			case 11:
				// -----------------------------------------------------------------
				// RAIL PRESSURE
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "FUEL");
				_sprintf(str,"%3d.%dMPa", (bcomp.p_fuel/1000), (bcomp.p_fuel/100)%10);
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_rail);
				} else {
					graph_puts32c(64, 24, str);
				}
				break;
			case 12:
				// -----------------------------------------------------------------
				// INTAKE PRESSURE
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "INTAKE");
				_sprintf(str,"%dkPa",bcomp.p_intake);
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_intake);
				} else {
					graph_puts32c(64, 24, str);
				}
				break;
			default:
				DBG("unknown page (%d)\r\n", bcomp.page);
				if (buttons & BUTT_SW2) {
					bcomp.page = 12;
				} else {
					bcomp.page = 1;
				}
				config_save(CPAR_PAGE, (uint8_t*)&bcomp.page, CPAR_PAGE_SIZE);
				goto repeate;
			}
		}
		// -----------------------------------------------------------------
		// Обновление экрана:
		// -----------------------------------------------------------------
		ms = get_ms_timer(); 
		graph_update(); 
		ms = get_ms_timer() - ms;
		DBG("graph_update() work %dms\r\n", ms);
#endif
		// Сохранение изменяемых параметров:
		if (save_flag & 0x01) {
			save_params();
			save_flag &= ~0x01;
		} else
		if (save_flag & 0x02) {
			save_settings();
			save_flag &= ~0x82;
		}
#if ( ELOG_SUPPORT == 1 )
		if (save_flag & 0x08) {
			elog_proc();
			save_flag &= ~0x08;
		}
#endif
	}
	return 0;
}

#if defined( WIN32 )
// Точка входа для отладочной WIN-версии:
int main(int argc, char **argv) {
	uint32_t addr;
	
	printf("-----------------------------------------------------------\r\n");
	printf("BCOMP11 Win32 PC build\r\n");
	printf("-----------------------------------------------------------\r\n");
	printf("A (or mouse click) - next, S - previous\r\n");
	printf("A long press - Enter, S long press - Cancel\r\n");
	printf("-----------------------------------------------------------\r\n");
	printf("\r\n");

	return lcd_init(ProcMain, "OLED", SIZE_X, SIZE_Y);
}
#endif
