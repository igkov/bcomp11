#include "i2c.h"
#include "dbg.h"

#if defined( CORTEX )

#ifndef BIT
#define BIT(a) (1<<(a))
#endif //BIT

#if defined( _LPC1700_ ) 
	#include <LPC17xx.h>
	#define I2C0SCLH    LPC_I2C0->I2SCLH
	#define I2C0SCLL    LPC_I2C0->I2SCLL
	#define I2C0CONSET  LPC_I2C0->I2CONSET
	#define I2C0CONCLR  LPC_I2C0->I2CONCLR
	#define I2C0STAT    LPC_I2C0->I2STAT
	#define I2C0DAT     LPC_I2C0->I2DAT
#elif defined( _LPC1300_ ) 
	#include <LPC13xx.h>
	#define I2C0SCLH    LPC_I2C->SCLH
	#define I2C0SCLL    LPC_I2C->SCLL
	#define I2C0CONSET  LPC_I2C->CONSET
	#define I2C0CONCLR  LPC_I2C->CONCLR
	#define I2C0STAT    LPC_I2C->STAT
	#define I2C0DAT     LPC_I2C->DAT
#elif defined( _LPC1100_ )
	#include <LPC11xx.h>
	#define I2C0SCLH    LPC_I2C->SCLH
	#define I2C0SCLL    LPC_I2C->SCLL
	#define I2C0CONSET  LPC_I2C->CONSET
	#define I2C0CONCLR  LPC_I2C->CONCLR
	#define I2C0STAT    LPC_I2C->STAT
	#define I2C0DAT     LPC_I2C->DAT
#endif

/* bit defines */
#define I2CON_AA			2
#define I2CON_SI			3
#define I2CON_STO			4
#define I2CON_STA			5
#define I2CON_I2EN			6

#else
	#include "hid.h"
	extern hid_context device;
#endif

int i2c_init(void) {
#if defined( CORTEX )
	LPC_SYSCON->PRESETCTRL    |= (0x1<<1); // reset i2c
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5);   // power on i2c

	LPC_IOCON->PIO0_4 &= ~0x3F; /* I2C I/O config */
	LPC_IOCON->PIO0_4 |=  0x01; /* I2C SCL */
	LPC_IOCON->PIO0_5 &= ~0x3F; /* I2C I/O config */
	LPC_IOCON->PIO0_5 |=  0x01; /* I2C SDA */ 
	// I2C_freq = VPBUS / (SCLH+SCLL)
	I2C0SCLH		= I2C_DIV_H;
	I2C0SCLL		= I2C_DIV_L;
	// disable and reset interface
	I2C0CONCLR = 0xFF;
	// enable interface
	I2C0CONSET = BIT(I2CON_I2EN);
	return 0;
#else
	int ret;
	if (device.hDeviceHandle == NULL) {
		ret = hid_find(&device, 0x1FC9, 0x0003, 0xFF00);
		if (ret == 0) {
			device.hDeviceHandle = NULL;
			return -1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
#endif
}

int i2c_deinit(void) {
#if defined( CORTEX )
	return 0;
#else
	hid_close(&device);
	return 0;
#endif
}

int i2c_write(uint8_t dev_addr, uint8_t addr, uint8_t *data, int len) {
#if defined( CORTEX )
	int i = 0;
	i2c(I2C_START, 0);
	if (i2c(I2C_ADDRESS, dev_addr&0xFE)!=I2C_MT_SLA_ACK) {
		i2c(I2C_STOP, 0);
		return I2C_ERROR;
	}
	i2c(I2C_TRANSMIT, addr);
	while (i < len) {
		i2c(I2C_TRANSMIT, data[i]);
		i++;
	}
	i2c(I2C_STOP, 0);
	return I2C_OK;
#else
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_I2C_WRITE;
	data_io->pc2dev.bDevAddr = dev_addr;
	data_io->pc2dev.bAddr = addr;
	data_io->pc2dev.bLen = len;
	memcpy(data_io->pc2dev.abData, data, len);
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0) {
		return -1;
	}
	
rep_read_answer_w:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0) {
		return -1;
	}
	if (data_io->dev2pc.bRetStatus == 0xFF) {
		goto rep_read_answer_w;
	}
	return data_io->dev2pc.bRetStatus;
#endif
}

int i2c_read(uint8_t dev_addr, uint8_t addr, uint8_t *data, int len) {
#if defined( CORTEX )
	int i = 0;
	i2c(I2C_START, 0);
	if (i2c(I2C_ADDRESS, dev_addr&0xFE)!=I2C_MT_SLA_ACK) {
		i2c(I2C_STOP, 0);
		return I2C_ERROR;
	}
	i2c(I2C_TRANSMIT, addr);
	i2c(I2C_RESTART, 0);
	if (i2c(I2C_ADDRESS, dev_addr|0x01)!=I2C_MR_SLA_ACK) {
		i2c(I2C_STOP, 0);
		return I2C_ERROR;
	}
	while (i < len) {
		if (i == len-1) {
			data[i] = i2c(I2C_RECEIVE_NACK, 0);
		} else {
			data[i] = i2c(I2C_RECEIVE_ACK, 0);
		}
		i++;
	}
	i2c(I2C_STOP, 0);
	return I2C_OK;
#else
	int ret;
	unsigned char buffer[64];
	io_data_t *data_io = (io_data_t*)buffer;
	
	data_io->pc2dev.bOperation = OP_I2C_READ;
	data_io->pc2dev.bDevAddr = dev_addr;
	data_io->pc2dev.bAddr = addr;
	data_io->pc2dev.bLen = len;
	
	ret = hid_write(&device, buffer, 64);
	if (ret == 0) {
		return -1;
	}

rep_read_answer_r:
	ret = hid_read(&device, buffer, 64);
	if (ret == 0) {
		return -1;
	}
	
	if (data_io->dev2pc.bRetStatus == 0xFF) {
		goto rep_read_answer_r;
	}
	
	memcpy(data, data_io->dev2pc.abData, data_io->dev2pc.bLen);
	return data_io->dev2pc.bRetStatus;
#endif
}

#if defined( CORTEX )
uint8_t i2c(uint8_t action, uint8_t data) {
	int n = 0;
	switch(action) {
	case I2C_START:
		I2C0CONSET = BIT(I2CON_STA);
		break;
	case I2C_RESTART:
		I2C0CONSET = BIT(I2CON_STA);
		I2C0CONCLR = BIT(I2CON_SI);
		break;
	case I2C_STOP: 
		I2C0CONSET = BIT(I2CON_STO);
		I2C0CONCLR = BIT(I2CON_SI);
		while( I2C0CONSET & BIT(I2CON_STO) ); // Wait for STOP detected
		for (; n < 10000; n++) { __NOP(); }
		return 0;
	case I2C_ADDRESS: 
		// save data into data register
		I2C0DAT = data;
		// clear SI bit to begin transfer
		I2C0CONCLR = BIT(I2CON_SI) | BIT(I2CON_STA);
		break;
	case I2C_TRANSMIT: 
		// save data into data register
		I2C0DAT = data;
		// clear SI bit to begin transfer
		I2C0CONCLR = BIT(I2CON_SI);
		break;
	case I2C_RECEIVE_ACK:
		// ackFlag = TRUE: ACK the recevied data
		I2C0CONSET = BIT(I2CON_AA);
		// clear SI bit to begin transfer
		I2C0CONCLR = BIT(I2CON_SI);
		break;
	case I2C_RECEIVE_NACK:
		// ackFlag = FALSE: NACK the recevied data
		I2C0CONCLR = BIT(I2CON_AA);
		// clear SI bit to begin transfer
		I2C0CONCLR = BIT(I2CON_SI);
		break;
	default:
		return I2C0STAT;
	}
	while (I2C0STAT == I2C_NO_INFO)
		if (n++ > 0x100000) break;
	switch (action)
	{
#if defined( _LPC214x_ )
	case I2C_START:
	case I2C_RESTART:
		I2C0CONCLR = BIT(I2CON_STA);
		return I2C0STAT;
#endif
	case I2C_RECEIVE_ACK:
	case I2C_RECEIVE_NACK:
		return I2C0DAT;
	default:
		return I2C0STAT;
	}
}
#endif
