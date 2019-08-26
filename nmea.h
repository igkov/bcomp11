#ifndef __NMEA_H__
#define __NMEA_H__

#include <stdint.h>

// Скорость идет в морских милях, требуется конвертация:
#define SPEED_COEFF    1.852

typedef struct {
	unsigned short year;
	unsigned char  month;
	unsigned char  date;
	unsigned char  hour;
	unsigned char  min;
	unsigned char  sec;
	unsigned char  wday;
} gpstime_t, *pgpstime_t;

// Тип приемника сигнала:
// Неизвестный тип приемника
#define NMEA_RECEIVER_UND     0
// GP - GPS
#define NMEA_RECEIVER_GPS     1
// GL - GLONASS
#define NMEA_RECEIVER_GLONASS 2

// Тип сообщения NMEA
#define NMEA_TYPE_UND         0
#define NMEA_TYPE_GGA         1
#define NMEA_TYPE_GLL         2
#define NMEA_TYPE_GSA         3
#define NMEA_TYPE_GSV         4
#define NMEA_TYPE_RMC         5
#define NMEA_TYPE_VTG         6
#define NMEA_TYPE_ZDA         7

/* UNIX time format" */
uint32_t time_to_unix(pgpstime_t op);
/* GPS parce functions: */
void nmea_parce(char *str);
int nmea_convert_coord_w(char *str, double *coord);
int nmea_convert_coord_l(char *str, double *coord);
/* Main Functions */
void nmea_proc(uint8_t ch);
void nmea_init(void);

#endif
