#ifndef __I2C_H__ 
#define __I2C_H__ 

#include <stdint.h>

int i2c_init(void);
int i2c_deinit(void);

#if defined( CORTEX )

/* Master Transmitter */
#define I2C_MT_SLA_ACK				0x18
#define I2C_MT_SLA_NACK				0x20
#define I2C_MT_DATA_ACK				0x28
#define I2C_MT_DATA_NACK			0x30
#define I2C_MT_ARB_LOST				0x38
/* Master Receiver */
#define I2C_MR_ARB_LOST				0x38
#define I2C_MR_SLA_ACK				0x40
#define I2C_MR_SLA_NACK				0x48
#define I2C_MR_DATA_ACK				0x50
#define I2C_MR_DATA_NACK			0x58
/* Misc */
#define I2C_NO_INFO					0xF8
#define I2C_BUS_ERROR				0x00

/**
	LPC2100
	I2C_freq = VPBUS / (SCLH+SCLL) = 60MHz / (80 + 80) = 375kHz
	LPC2300
	I2C_freq = VPBUS / (SCLH+SCLL) = 60MHz / (80 + 80) = 375kHz
	LPC1700:
	I2C_freq = VPBUS / (SCLH+SCLL) = 48MHz / (80 + 80) = 300kHz
	LPC1300:
	I2C_freq = VPBUS / (SCLH+SCLL) = 72MHz / (360 + 360) = 100kHz
	I2C_freq = VPBUS / (SCLH+SCLL) = 72MHz / (180 + 180) = 200kHz
	LPC1100
	I2C_freq = VPBUS / (SCLH+SCLL) = 48MHz / (90 + 90) = 266kHz
	I2C_freq = VPBUS / (SCLH+SCLL) = 48MHz / (60 + 60) = 400kHz
	I2C_freq = VPBUS / (SCLH+SCLL) = 48MHz / (48 + 48) = 500kHz
	I2C_freq = VPBUS / (SCLH+SCLL) = 48MHz / (24 + 24) = 1MHz
 */
#define I2C_DIV_H 90
#define I2C_DIV_L 90

#define I2C_OK           0x00
#define I2C_ERROR        0x01
#define I2C_OVF_BUFFER   0x02

#define I2C_START        0x01
#define I2C_RESTART      0x02
#define I2C_STOP         0x03
#define I2C_ADDRESS      0x04
#define I2C_TRANSMIT     0x05
#define I2C_RECEIVE_ACK  0x06
#define I2C_RECEIVE_NACK 0x07

uint8_t i2c(uint8_t stat, uint8_t data);
#endif

int i2c_write(uint8_t dev_addr, uint8_t addr, uint8_t *buffer, int len);
int i2c_read(uint8_t dev_addr, uint8_t addr, uint8_t *buffer, int len);

#endif
