#include <string.h>
#include <lpc11xx.h>
#include "bcomp.h"
#include "dbg.h"
#include "i2c.h"
#include "eeprom.h"
#define IS_2B_ADDRESS (bconfig.ee_size>16)

#if !defined( EEPROM_STD_ADDRESS )
#include "spi.h"

unsigned char ee_getstat(void) {
	unsigned char stat;
	spi_select();
	spi_send(SPI_RDSR);
	stat = spi_send(0xFF);
	spi_unselect();
	return stat;
}

unsigned char ee_setstat(unsigned char stat) {
	spi_select();
	spi_send(SPI_WRSR);
	spi_send(stat);
	spi_unselect();
	return stat;
}

void ee_write_dis(void) {
	spi_select();
	spi_send(SPI_WRDI);
	spi_unselect();
}

void ee_write_en(void) {
	spi_select();
	spi_send(SPI_WREN);
	spi_unselect();
}
#endif

unsigned char ee_write(unsigned short addr, unsigned char *buff, unsigned char len) {
	int offset = 0;
	register int d = 20000;
	unsigned char addr_hw;
#if defined( EEPROM_STD_ADDRESS )
	i2c(I2C_START, 0);
	addr_hw = (EEPROM_STD_ADDRESS&0xFE);
	if (!IS_2B_ADDRESS) {
		addr_hw |= ((addr>>7)&0x0E);
	}
	if (i2c(I2C_ADDRESS, addr_hw)!=I2C_MT_SLA_ACK) {
		i2c(I2C_STOP, 0);
		return I2C_ERROR;
	}
	if (IS_2B_ADDRESS) {
		i2c(I2C_TRANSMIT, addr>>8);
	}
	i2c(I2C_TRANSMIT, addr&0xFF);
	while (offset < len) {
		i2c(I2C_TRANSMIT, buff[offset]);
		offset++;
	}
	i2c(I2C_STOP, 0);
	while (d--) __NOP();
	return I2C_OK;
#else
	ee_write_en();
	spi_select();
	spi_send(SPI_WRITE);
	spi_send(addr>>8);
	spi_send(addr&0xFF);
	for (offset = 0; offset < len; offset++)
		spi_send(buff[offset]);
	spi_unselect();
	while ( ee_getstat() & 0x01 );
	ee_write_dis();
	return I2C_OK;
#endif
}

unsigned char ee_read(unsigned short addr, unsigned char *buff, unsigned char len) {
	int offset = 0;
	unsigned char addr_hw;
#if defined( EEPROM_STD_ADDRESS )
	i2c(I2C_START, 0);
	addr_hw = (EEPROM_STD_ADDRESS&0xFE);
	if (!IS_2B_ADDRESS) {
		addr_hw |= ((addr>>7)&0x0E);
	}
	if (i2c(I2C_ADDRESS, addr_hw)!=I2C_MT_SLA_ACK) {
		i2c(I2C_STOP, 0);
		return I2C_ERROR;
	}
	if (IS_2B_ADDRESS) {
		i2c(I2C_TRANSMIT, addr>>8);
	}
	i2c(I2C_TRANSMIT, addr&0xFF);
	i2c(I2C_RESTART, 0);
	if (i2c(I2C_ADDRESS, addr_hw|0x01)!=I2C_MR_SLA_ACK) {
		i2c(I2C_STOP, 0);
		return I2C_ERROR;
	}
	while (offset < len) {
		if (offset == len-1) {
			buff[offset] = i2c(I2C_RECEIVE_NACK, 0);
		} else {
			buff[offset] = i2c(I2C_RECEIVE_ACK, 0);
		}
		offset++;
	}
	i2c(I2C_STOP, 0);
	return I2C_OK;
#else
	spi_select();
	spi_send(SPI_READ);
	spi_send(addr>>8);
	spi_send(addr&0xFF);
	for (offset = 0; offset < len; offset++)
		buff[offset] = spi_send(0xFF);
	spi_unselect();
	return I2C_OK;
#endif
}

void ee_clear(void) {
	unsigned char data[16];
	unsigned short addr;
	#define EE_SIZE 0x400
	memset(data, 0xFF, sizeof(data));
	for (addr = 0; addr < EE_SIZE; addr += sizeof(data)) {
		ee_write(addr, data, sizeof(data));
		
	}
}

int ee_size_detect(void) {
	// 
	// experimental function for detect I2C protocol type (1 or 2 bytes address).
	// 
}
