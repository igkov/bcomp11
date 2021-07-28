#ifndef __BCOMP_H__
#define __BCOMP_H__

#include <stdint.h>

#define GUI_FLAG_MENU    0x1000
#define GUI_FLAG_WARNING 0x2000
#define GUI_FLAG_GRAPH   0x4000

#if !defined( WIN32 )
extern int melody_warning[];
extern int melody_start[];
extern int melody_wrep[];
extern int melody_wrep2[];
#endif

#if defined( CFG_PAJERO )

// PAJERO CONFIGURATION:

// Поддержка протокола Virtuino:
#define VIRTUINO_SUPPORT 0
// Флаг сборки для встраиваемой версии (другой тип экрана):
#define INSIDE_VERSION 0
// Поддержка графического интерфейса:
#define GRAPH_SUPPORT 1
// Поддержка вывода во внешний лог-файл:
#define ELOG_SUPPORT 0
// Минимальная поддержка nmea:
#define NMEA_SUPPORT 0
// Поддержка предупреждений:
#define WARNING_SUPPORT 0
// Специфичные возможности шины Mitsubishi Pajero Sport II:
#define PAJERO_SPECIFIC 0
// Специфичные возможности шины Nissan:
#define NISSAN_SPECIFIC 1
// Тип кнопок:
#define BUTTONS_ANALOG 0
// Поддержка OLED на контроллере SSD1306:
#define OLED_SSD1306_SUPPORT 1
// Поддержка OLED на контроллере SH1106:
#define OLED_SH1106_SUPPORT 0
// Поддержка экрана с выводом положения колес:
#define WHELLS_DRAW_SUPPORT 0

#elif defined( CFG_NISSAN )

// NISSAN CONFIGURATION:

// Поддержка протокола Virtuino:
#define VIRTUINO_SUPPORT 1
// Флаг сборки для встраиваемой версии (другой тип экрана):
#define INSIDE_VERSION 0
// Поддержка графического интерфейса:
#define GRAPH_SUPPORT 1
// Поддержка вывода во внешний лог-файл:
#define ELOG_SUPPORT 0
// Минимальная поддержка nmea:
#define NMEA_SUPPORT 0
// Поддержка предупреждений:
#define WARNING_SUPPORT 0
// Специфичные возможности шины Mitsubishi Pajero Sport II:
#define PAJERO_SPECIFIC 0
// Специфичные возможности шины Nissan:
#define NISSAN_SPECIFIC 1
// Тип кнопок:
#define BUTTONS_ANALOG 0
// Поддержка OLED на контроллере SSD1306:
#define OLED_SSD1306_SUPPORT 1
// Поддержка OLED на контроллере SH1106:
#define OLED_SH1106_SUPPORT 1
// Поддержка экрана с выводом положения колес:
#define WHELLS_DRAW_SUPPORT 0

#elif defined( CFG_VIRTUINO ) || defined( CFG_GCC )

// GCC BUILD CONFIGURATION (VIRTUINO ONLY):

// Поддержка протокола Virtuino:
#define VIRTUINO_SUPPORT 1
// Флаг сборки для встраиваемой версии (другой тип экрана):
#define INSIDE_VERSION 0
// Поддержка графического интерфейса:
#define GRAPH_SUPPORT 0
// Поддержка вывода во внешний лог-файл:
#define ELOG_SUPPORT 0
// Минимальная поддержка nmea:
#define NMEA_SUPPORT 0
// Поддержка предупреждений:
#define WARNING_SUPPORT 0
// Специфичные возможности шины Mitsubishi Pajero Sport II:
#define PAJERO_SPECIFIC 0
// Специфичные возможности шины Nissan:
#define NISSAN_SPECIFIC 0
// Тип кнопок:
#define BUTTONS_ANALOG 0
// Поддержка OLED на контроллере SSD1306:
#define OLED_SSD1306_SUPPORT 0
// Поддержка OLED на контроллере SH1106:
#define OLED_SH1106_SUPPORT 0
// Поддержка экрана с выводом положения колес:
#define WHELLS_DRAW_SUPPORT 0

#else

#error Unknown main configuration!

#endif

#if ( ELOG_SUPPORT == 1 ) && ( VIRTUINO_SUPPORT == 1 ) 
#error Unsupport ELOG and VIRTUINO simultaneously.
#endif

#if ( NMEA_SUPPORT == 1 ) && ( VIRTUINO_SUPPORT == 1 ) 
#error Unsupport NMEA and VIRTUINO simultaneously.
#endif

#if ( PAJERO_SPECIFIC == 1 ) && ( NISSAN_SPECIFIC == 1 )
#error Conflict in simultaneously define PAJERO and NISSAN capabilities.
#endif

// Определения для экрана информации:
#if ( PAJERO_SPECIFIC == 1 )
#define INFO_DEVICE   "NMPS COMP"
#elif ( NISSAN_SPECIFIC == 1 )
#define INFO_DEVICE   "NISSAN-CO"
#else
#define INFO_DEVICE   "BCOMP11"
#endif
#define INFO_VERSION  "v2.3a"
#define INFO_AUTHOR   "igorkov"
#define INFO_YEAR     "2019"
// Максимальная длина строки на отображение: 10 символов!

#include "analog.h"
#include "nmea.h"
#include "lowpass.h"
#include "diagram.h"

typedef struct {
	uint32_t time;   // Время маршрута (сек).
	double dist;     // Дистанция маршрута (м).
	double fuel;     // Рассчет топлива за поездку (мл).
} trip_t, *ptrip_t;  // 20b

typedef struct {
	float fuel;
	float dist;
} pars_t, *ppars_t;

typedef struct {
	uint32_t time;                // Время работы.
    uint8_t  can_act;             // Флаг активности работы.
    uint8_t  res0[3];             // Выравнивание.

	struct {
		float v_max;              // Максимально допустимое напряжение бортовой сети (Вольт).
		float v_min;              // Минимально допустимое напряжение бортовой сети (Вольт).
		float fuel_cal;           // Калибровочная константа потребления топлива.
		float speed_cal;          // Калибровочная константа скорости.
		int t_at;                 // Максимально допустимая температура АКПП (°C).
		int t_eng;                // Максимально допустимая температура двигателя (°C).
		int t_ext;                // Граничная температура предупреждения о гололеде (°C).
		int w_delay;              // Задержка повторения предупреждения (в минутах).
		int f_gps;                // Флаг присутсвия GPS-приемника в системе.
		int i_gps;                // Настройка скорости GPS-приемника (бодд).
		//int time;                 // Смещение часового пояса.
		int f_fuel;               // Флаг наличия подключения к ДУТ.
		float l_fuel;             // Граничное значение топлива в баке по ДУТ (л).
		int f_ext;                // Флал наличия датчика внешней температуры.
		int f_ext_w;              // Флаг блокировки предупреждения о гололеде.
		int f_esp;                // Флаг наличия системы курсовой устойчивости (ACS, ESP, ESC).
		int f_log;                // Флаг надобности вывода лога.
		int contrast;             // Значение уровня контраста/яркости.
		int sound;                // Активация звуков.
	} setup;

	int page;                     // Страница отображения.

	int32_t press;
	int32_t temp;
    float mmhg;
	float height;
	filter_t filt_p;

	diagram_t dia_press;         // Данные диаграммы температуры двигателя.
	diagram_t dia_height;          // Данные диаграммы температуры коробки.
	diagram_t dia_temp;          // Данные диаграммы температуры коробки.
} bcomp_t;

/*
	Актуальная версия структуры конфигурации:
 */
#define CONFIGURATON_VERSION 103

typedef struct {
	uint32_t begin_label;         // Метка начала структуры конфигурации (для поиска в памяти).
	uint32_t version;             // Версия текущей структуры.

	int can_speed;                // Скорость CAN-интерфейса.
	int uart_speed;               // Скорость UART-интерфейса.
	uint8_t ee_size;              // Объем микросхемы EEPROM памяти.
	uint8_t contrast;             // Значение контраста экрана.
	uint8_t start_delay;          // Кол-во секунд, сколько показывать заставку.
	uint8_t start_sound;          // Делать ли звуковое оповещение при старте.
	int t_engine_warning;         // Температура предупрежения о перегреве двигателя.
	int t_akpp_warning;           // Температура предупрежения о перегреве коробки.
	float v_max;                  // Максимальное напряжение бортовой сети.
	float v_min;                  // Минимальное напряжение бортовой сети.
	float fuel_coeff;             // Коэффициент пересчета для потребления топлива.
	float speed_coeff;            // Коэффициент пересчета для рассчета скорости.
	uint32_t moto_dist_offset;    // Стартовое значение счетчика километров.
	uint32_t moto_time_offset;    // Стартовое значение счетчика моточасов.
	uint32_t service_distation;   // Межсервисный интервал (км).
	uint32_t service_motohour;    // Межсервисный интервал (моточасы).

	uint8_t oled_type;            // Тип контроллера OLED-экрана (0 - OLED SSD1306, 1 - OLED SH1106)
	uint8_t elog_flag;            // Флаг вывода лога в UART-порт.
	uint8_t res2;                 // ...
	uint8_t res3;                 // ...
	
	termist_t termistor;          // Настройка терморезистора.
	
	uint32_t end_label;           // Завершающая метка структуры (для проверки размера).
} bcomp_config_t;

typedef struct {
	uint32_t begin_label;
	uint32_t version;

	int can_speed;
	uint8_t ee_size;
	uint8_t contrast;
	uint8_t start_delay;
	uint8_t start_sound;
	int t_engine_warning;
	int t_akpp_warning;
	float v_max;
	float v_min;
	float fuel_coeff;
	float speed_coeff;
	uint32_t moto_dist_offset;
	uint32_t moto_time_offset;
	
	uint32_t end_label;
} bcomp_config102_t;

typedef struct {
	uint32_t begin_label;
	uint32_t version;

	int can_speed;
	int t_engine_warning;
	int t_akpp_warning;
	float v_max;
	float v_min;
	float fuel_coeff;
	float speed_coeff;
	uint32_t moto_dist_offset;
	uint32_t moto_time_offset;
	
	uint32_t end_label;
} bcomp_config101_t;

typedef struct {
	uint32_t begin_label;
	uint32_t version;

	int can_speed;
	int t_engine_warning;
	int t_akpp_warning;
	float v_max;
	float v_min;
	float fuel_coeff;
	float speed_coeff;
	
	uint32_t end_label;
} bcomp_config100_t;

extern bcomp_t bcomp;
extern const bcomp_config_t bconfig;

void bcomp_proc(int pid, uint8_t *data, uint8_t size);
void bcomp_raw(int pid, uint8_t *data, uint8_t size);

#endif
