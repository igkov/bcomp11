#ifndef __ICONS_H__
#define __ICONS_H__

#include <stdint.h>

#if !defined( NULL )
#define NULL ((void*)0)
#endif

/*
	Поле размера используется для функционала, когда структуры запакованы!
 */
typedef struct {
	uint8_t sizex;
	uint8_t sizey;
	uint16_t size;
	const uint8_t *data;
} ico_t, *pico_t;

// Иконки (не сжатые):
extern const uint8_t ico16_100kms_data[];
extern const uint8_t ico16_at_data[];
extern const uint8_t ico16_battery_data[];
extern const uint8_t ico16_engine_data[];
extern const uint8_t ico16_fuel_data[];
extern const uint8_t ico16_km_data[];
extern const uint8_t ico16_kmh_data[];
extern const uint8_t ico16_time_data[];
extern const uint8_t ico16_temp_data[];
extern const uint8_t ico16_mt_data[];
extern const uint8_t ico16_road_data[];
extern const uint8_t ico16_rpm_data[];
extern const uint8_t ico16_warning_data[];
//extern const uint8_t ico64_mitsu_data[];

// Иконки (сжатые):
extern const ico_t ico64_mitsu;
extern const ico_t ico64_nissan;

extern const ico_t ico48_engine;
extern const ico_t ico48_silkroad;
extern const ico_t ico48_temp;
extern const ico_t ico48_battery;
extern const ico_t ico48_trans;
extern const ico_t ico48_connect;
extern const ico_t ico48_gps;
extern const ico_t ico48_service;

extern const ico_t ico32_at_p;
extern const ico_t ico32_at_n;
extern const ico_t ico32_at_r;
extern const ico_t ico32_at_d1;
extern const ico_t ico32_at_d2;
extern const ico_t ico32_at_d3;
extern const ico_t ico32_at_d4;
extern const ico_t ico32_at_d5;

#endif //__ICONS_H__
