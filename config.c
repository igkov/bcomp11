#include <string.h>
#include "eeprom.h"
#include "config.h"

#if !defined( WIN32 )
#include <lpc11xx.h>
#else
#define __NOP()
#endif

extern unsigned char crc8(unsigned char *block, int len);

int config_read(int param, uint8_t *value, int len) {
	config_unit_t unit;
	uint32_t addr = 2*param*sizeof(config_unit_t);
	if (param >= CONFIG_MAX_PARAM ||
		param < 0)
		return 1;
	// 1ое чтение:
	ee_read(addr, (uint8_t*)&unit, sizeof(config_unit_t));
	if (unit.flag == 0xDE &&
		unit.len >= len &&
		unit.crc8 == crc8(unit.data, len)) {
		memcpy(value, unit.data, len);
		return 0;
	}
	// Не прочитано. 2ое чтение:
	ee_read(addr+sizeof(config_unit_t), (uint8_t*)&unit, sizeof(config_unit_t));
	if (unit.flag == 0xEA &&
		unit.len >= len &&
		unit.crc8 == crc8(unit.data, len)) {
		memcpy(value, unit.data, len);
		return 0;
	}
	return 2;
}

int config_save(int param, uint8_t *value, int len) {
	config_unit_t unit;
	uint32_t addr = 2*param*sizeof(config_unit_t);
	int n;
	if (param >= CONFIG_MAX_PARAM ||
		param < 0)
		return 1;
	if (len > CONFIG_MAX_SIZE)
		return 2;
	unit.crc8 = crc8(value, len);
	unit.len = len;
	memcpy(unit.data, value, len);
	// 1ая запись:
	unit.flag = 0xDE;
	ee_write(addr, (uint8_t*)&unit, sizeof(config_unit_t));
	// NOTE: данная задержка актуальна для i2c-памяти, 
	// т.к. мы имеем ошибки с недозаписью. Причем в силу архитектуры 
	// конфига не проявляемые явно.
	for (n=0; n<30000; n++) {
		__NOP();
	}
	// 2ая запись:
	unit.flag = 0xEA;
	ee_write(addr+sizeof(config_unit_t), (uint8_t*)&unit, sizeof(config_unit_t));
	// NOTE: данная задержка актуальна для i2c-памяти, 
	// т.к. мы имеем ошибки с недозаписью. Причем в силу архитектуры 
	// конфига не проявляемые явно.
	for (n=0; n<30000; n++) {
		__NOP();
	}
	return 0;
}
