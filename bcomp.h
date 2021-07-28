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

// ��������� ��������� Virtuino:
#define VIRTUINO_SUPPORT 0
// ���� ������ ��� ������������ ������ (������ ��� ������):
#define INSIDE_VERSION 0
// ��������� ������������ ����������:
#define GRAPH_SUPPORT 1
// ��������� ������ �� ������� ���-����:
#define ELOG_SUPPORT 0
// ����������� ��������� nmea:
#define NMEA_SUPPORT 0
// ��������� ��������������:
#define WARNING_SUPPORT 0
// ����������� ����������� ���� Mitsubishi Pajero Sport II:
#define PAJERO_SPECIFIC 0
// ����������� ����������� ���� Nissan:
#define NISSAN_SPECIFIC 1
// ��� ������:
#define BUTTONS_ANALOG 0
// ��������� OLED �� ����������� SSD1306:
#define OLED_SSD1306_SUPPORT 1
// ��������� OLED �� ����������� SH1106:
#define OLED_SH1106_SUPPORT 0
// ��������� ������ � ������� ��������� �����:
#define WHELLS_DRAW_SUPPORT 0

#elif defined( CFG_NISSAN )

// NISSAN CONFIGURATION:

// ��������� ��������� Virtuino:
#define VIRTUINO_SUPPORT 1
// ���� ������ ��� ������������ ������ (������ ��� ������):
#define INSIDE_VERSION 0
// ��������� ������������ ����������:
#define GRAPH_SUPPORT 1
// ��������� ������ �� ������� ���-����:
#define ELOG_SUPPORT 0
// ����������� ��������� nmea:
#define NMEA_SUPPORT 0
// ��������� ��������������:
#define WARNING_SUPPORT 0
// ����������� ����������� ���� Mitsubishi Pajero Sport II:
#define PAJERO_SPECIFIC 0
// ����������� ����������� ���� Nissan:
#define NISSAN_SPECIFIC 1
// ��� ������:
#define BUTTONS_ANALOG 0
// ��������� OLED �� ����������� SSD1306:
#define OLED_SSD1306_SUPPORT 1
// ��������� OLED �� ����������� SH1106:
#define OLED_SH1106_SUPPORT 1
// ��������� ������ � ������� ��������� �����:
#define WHELLS_DRAW_SUPPORT 0

#elif defined( CFG_VIRTUINO ) || defined( CFG_GCC )

// GCC BUILD CONFIGURATION (VIRTUINO ONLY):

// ��������� ��������� Virtuino:
#define VIRTUINO_SUPPORT 1
// ���� ������ ��� ������������ ������ (������ ��� ������):
#define INSIDE_VERSION 0
// ��������� ������������ ����������:
#define GRAPH_SUPPORT 0
// ��������� ������ �� ������� ���-����:
#define ELOG_SUPPORT 0
// ����������� ��������� nmea:
#define NMEA_SUPPORT 0
// ��������� ��������������:
#define WARNING_SUPPORT 0
// ����������� ����������� ���� Mitsubishi Pajero Sport II:
#define PAJERO_SPECIFIC 0
// ����������� ����������� ���� Nissan:
#define NISSAN_SPECIFIC 0
// ��� ������:
#define BUTTONS_ANALOG 0
// ��������� OLED �� ����������� SSD1306:
#define OLED_SSD1306_SUPPORT 0
// ��������� OLED �� ����������� SH1106:
#define OLED_SH1106_SUPPORT 0
// ��������� ������ � ������� ��������� �����:
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

// ����������� ��� ������ ����������:
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
// ������������ ����� ������ �� �����������: 10 ��������!

#include "analog.h"
#include "nmea.h"
#include "lowpass.h"
#include "diagram.h"

typedef struct {
	uint32_t time;   // ����� �������� (���).
	double dist;     // ��������� �������� (�).
	double fuel;     // ������� ������� �� ������� (��).
} trip_t, *ptrip_t;  // 20b

typedef struct {
	float fuel;
	float dist;
} pars_t, *ppars_t;

typedef struct {
	uint32_t time;                // ����� ������.
    uint8_t  can_act;             // ���� ���������� ������.
    uint8_t  res0[3];             // ������������.

	struct {
		float v_max;              // ����������� ���������� ���������� �������� ���� (�����).
		float v_min;              // ���������� ���������� ���������� �������� ���� (�����).
		float fuel_cal;           // ������������� ��������� ����������� �������.
		float speed_cal;          // ������������� ��������� ��������.
		int t_at;                 // ����������� ���������� ����������� ���� (�C).
		int t_eng;                // ����������� ���������� ����������� ��������� (�C).
		int t_ext;                // ��������� ����������� �������������� � �������� (�C).
		int w_delay;              // �������� ���������� �������������� (� �������).
		int f_gps;                // ���� ���������� GPS-��������� � �������.
		int i_gps;                // ��������� �������� GPS-��������� (����).
		//int time;                 // �������� �������� �����.
		int f_fuel;               // ���� ������� ����������� � ���.
		float l_fuel;             // ��������� �������� ������� � ���� �� ��� (�).
		int f_ext;                // ���� ������� ������� ������� �����������.
		int f_ext_w;              // ���� ���������� �������������� � ��������.
		int f_esp;                // ���� ������� ������� �������� ������������ (ACS, ESP, ESC).
		int f_log;                // ���� ���������� ������ ����.
		int contrast;             // �������� ������ ���������/�������.
		int sound;                // ��������� ������.
	} setup;

	int page;                     // �������� �����������.

	int32_t press;
	int32_t temp;
    float mmhg;
	float height;
	filter_t filt_p;

	diagram_t dia_press;         // ������ ��������� ����������� ���������.
	diagram_t dia_height;          // ������ ��������� ����������� �������.
	diagram_t dia_temp;          // ������ ��������� ����������� �������.
} bcomp_t;

/*
	���������� ������ ��������� ������������:
 */
#define CONFIGURATON_VERSION 103

typedef struct {
	uint32_t begin_label;         // ����� ������ ��������� ������������ (��� ������ � ������).
	uint32_t version;             // ������ ������� ���������.

	int can_speed;                // �������� CAN-����������.
	int uart_speed;               // �������� UART-����������.
	uint8_t ee_size;              // ����� ���������� EEPROM ������.
	uint8_t contrast;             // �������� ��������� ������.
	uint8_t start_delay;          // ���-�� ������, ������� ���������� ��������.
	uint8_t start_sound;          // ������ �� �������� ���������� ��� ������.
	int t_engine_warning;         // ����������� ������������� � ��������� ���������.
	int t_akpp_warning;           // ����������� ������������� � ��������� �������.
	float v_max;                  // ������������ ���������� �������� ����.
	float v_min;                  // ����������� ���������� �������� ����.
	float fuel_coeff;             // ����������� ��������� ��� ����������� �������.
	float speed_coeff;            // ����������� ��������� ��� �������� ��������.
	uint32_t moto_dist_offset;    // ��������� �������� �������� ����������.
	uint32_t moto_time_offset;    // ��������� �������� �������� ���������.
	uint32_t service_distation;   // ������������ �������� (��).
	uint32_t service_motohour;    // ������������ �������� (��������).

	uint8_t oled_type;            // ��� ����������� OLED-������ (0 - OLED SSD1306, 1 - OLED SH1106)
	uint8_t elog_flag;            // ���� ������ ���� � UART-����.
	uint8_t res2;                 // ...
	uint8_t res3;                 // ...
	
	termist_t termistor;          // ��������� ��������������.
	
	uint32_t end_label;           // ����������� ����� ��������� (��� �������� �������).
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
