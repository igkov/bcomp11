#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

// Модуль реализует защищенную запись в EEPROM. Каждый параметр хранится в 2х ячейках.
// Это позволяет при повреждении данных в момент записи в одной из ячеек, при последующем 
// чтении гарантированно восстановить корректное значение из другой ячейки.

// 64 * 16 * 2 = 2048Б
#define CONFIG_MAX_PARAM 64
#define CONFIG_MAX_SIZE  13
#define CONFIG_OFFSET    (0x0000)
#define CONFIG_SIZE      (2*CONFIG_MAX_PARAM*sizeof(config_unit_t))

typedef struct {
	uint8_t flag;
	uint8_t len;
	uint8_t crc8;
	uint8_t data[CONFIG_MAX_SIZE];
} config_unit_t, *pconfig_unit_t;

// Последний экран:
#define CPAR_PAGE                1
#define CPAR_PAGE_SIZE           4
// Глобальный счетчик пробега:
#define CPAR_DIST_GLOB           2
#define CPAR_DIST_GLOB_SIZE      8
// Глобальный счетчик моточасов:
#define CPAR_MOTO_GLOB           3
#define CPAR_MOTO_GLOB_SIZE      4
// Сервисный счетчик пробега:
#define CPAR_DIST_SERV           4
#define CPAR_DIST_SERV_SIZE      8
// Сервисный счетчик моточасов:
#define CPAR_MOTO_SERV           5
#define CPAR_MOTO_SERV_SIZE      4
// Путь А: Дистанция
#define CPAR_TRIPA_DIST          6
#define CPAR_TRIPA_DIST_SIZE     8
// Путь А: Время
#define CPAR_TRIPA_TIME          7
#define CPAR_TRIPA_TIME_SIZE     8
// Путь А: Топливо
#define CPAR_TRIPA_FUEL          8
#define CPAR_TRIPA_FUEL_SIZE     8
// Путь Б: Дистанция
#define CPAR_TRIPB_DIST          9
#define CPAR_TRIPB_DIST_SIZE     8
// Путь Б: Время+Топливо
#define CPAR_TRIPB_TIME          10
#define CPAR_TRIPB_TIME_SIZE     8
// Путь Б: Топливо
#define CPAR_TRIPB_FUEL          11
#define CPAR_TRIPB_FUEL_SIZE     8
// Одометр:
//#define CPAR_SETUP_ODO           12
//#define CPAR_SETUP_ODO_SIZE      4
// Контраст экрана:
#define CPAR_CONTRAST            13
#define CPAR_CONTRAST_SIZE       4
// Флаг сервисного режима:
#define CPAR_SERVICE             14
#define CPAR_SERVICE_SIZE        1
// Флаг сервисного режима:
#define CPAR_DATE_SERV           15
#define CPAR_DATE_SERV_SIZE      13
// Переменная уровня топлива в баке:
#define CPAR_FUEL_LEVEL          16
#define CPAR_FUEL_LEVEL_SIZE     1

// Максимальное напряжение
#define CPAR_SETUP_V_MAX         20
#define CPAR_SETUP_V_MAX_SIZE    4
#define CPAR_SETUP_V_MIN         21
#define CPAR_SETUP_V_MIN_SIZE    4
#define CPAR_SETUP_T_AT          22
#define CPAR_SETUP_T_AT_SIZE     4
#define CPAR_SETUP_T_ENG         23
#define CPAR_SETUP_T_ENG_SIZE    4
//#define CPAR_SETUP_F_FUEL        24
//#define CPAR_SETUP_F_FUEL_SIZE   1
//#define CPAR_SETUP_L_FUEL        25
//#define CPAR_SETUP_L_FUEL_SIZE   4
//#define CPAR_SETUP_TIME          26
//#define CPAR_SETUP_TIME_SIZE     1
#define CPAR_SETUP_W_DELAY       27
#define CPAR_SETUP_W_DELAY_SIZE  4
#define CPAR_SETUP_F_EXT         28
#define CPAR_SETUP_F_EXT_SIZE    1
#define CPAR_SETUP_F_EXT_W       29
#define CPAR_SETUP_F_EXT_W_SIZE  1
#define CPAR_SETUP_T_EXT         30
#define CPAR_SETUP_T_EXT_SIZE    4
#define CPAR_SETUP_F_GPS         31
#define CPAR_SETUP_F_GPS_SIZE    4
#define CPAR_SETUP_I_GPS         32
#define CPAR_SETUP_I_GPS_SIZE    4
#define CPAR_SETUP_F_ESP         33
#define CPAR_SETUP_F_ESP_SIZE    1
#define CPAR_SETUP_FUEL_CAL      34
#define CPAR_SETUP_FUEL_CAL_SIZE 4
#define CPAR_SETUP_F_LOG         35
#define CPAR_SETUP_F_LOG_SIZE    1

int config_read(int param, uint8_t *value, int len);
int config_save(int param, uint8_t *value, int len);

#endif
