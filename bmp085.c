#include "bmp085.h"
#include "i2c.h"
#include "rev.h"

static struct
{
	int16_t AC1;
	int16_t AC2;
	int16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;
} bmp085_coeff;

#define I2C_BMP085_ADDR 0xEE

static uint8_t flag;
//static int32_t press;
//static int16_t temp;
static uint8_t OSS;
static int32_t B5;

int bmp085_init(void)
{
	int ret;
	
	ret = i2c_read(I2C_BMP085_ADDR, 0xAA, (unsigned char*)&bmp085_coeff, sizeof(bmp085_coeff));
	if (ret)
		return ret;

	rev16((uint16_t*)&bmp085_coeff.AC1);
	rev16((uint16_t*)&bmp085_coeff.AC2);
	rev16((uint16_t*)&bmp085_coeff.AC3);
	rev16((uint16_t*)&bmp085_coeff.AC4);
	rev16((uint16_t*)&bmp085_coeff.AC5);
	rev16((uint16_t*)&bmp085_coeff.AC6);
	rev16((uint16_t*)&bmp085_coeff.B1);
	rev16((uint16_t*)&bmp085_coeff.B2);
	rev16((uint16_t*)&bmp085_coeff.MB);
	rev16((uint16_t*)&bmp085_coeff.MC);
	rev16((uint16_t*)&bmp085_coeff.MD);

	flag = 0;
	
	return ret;
}

int bmp085_start_temp(void)
{
	int ret;
	uint8_t data[4];
	// Baro Read Raw Temperature:
	data[0] = 0x2E;
	ret = i2c_write(I2C_BMP085_ADDR, 0xF4, data, 0x01);
	if (ret == 0)
        flag = 1;
	return ret;
}

int bmp085_get_temp(int32_t *t)
{
	int ret;
	int32_t UT, X1, X2, T;
	uint8_t data[4];

	if (flag == 1)
	{
		ret = i2c_read(I2C_BMP085_ADDR, 0xF6, data, 0x02);
		if (ret)
			return ret;
		UT = (data[0] << 8) + data[1];
		
		X1 = ((UT - (int32_t)bmp085_coeff.AC6) * (int32_t)bmp085_coeff.AC5) >> 15;
		X2 = ((int32_t)bmp085_coeff.MC << 11) / (X1 + (int32_t)bmp085_coeff.MD);
		B5 = X1 + X2;
		T = (B5 + 8) >> 4;
		
		//DBG("Temperature = %d (0.1*C), ", T);
		
		*t = T;
		return 0;
	}
	else
	{
		return -1;
	}
}

int bmp085_start_baro(int prec)
{
	int ret;
	uint8_t data[4];

	OSS = prec;
	// Start Command
	data[0] = 0x34 + (OSS<<6);
	ret = i2c_write(I2C_BMP085_ADDR, 0xF4, data, 0x01);
	if (ret == 0)
		flag = 2;
	return ret;
}

int bmp085_get_baro(int32_t *p)
{
	int ret;

	int32_t UP;
	int32_t X1, X2;
	int32_t B6, X3, B3;
	uint32_t B4, B7;
	int32_t P;

	uint8_t data[4];

	if (flag == 2)
	{
		// Baro Read Raw Pressure
		ret = i2c_read(I2C_BMP085_ADDR, 0xF6, data, 0x03);
		if (ret)
			return ret;
		
		UP = ((data[0] << 16) + (data[1] << 8) + data[2]) >> (8 - OSS);
		
		B6 = B5 - 4000L;
		X1 = ((int32_t)bmp085_coeff.B2 * ((B6 * B6) >> 12)) >> 11;
		X2 = ((int32_t)bmp085_coeff.AC2 * B6) >> 11;
		X3 = X1 + X2;
		B3 = ((((int32_t)bmp085_coeff.AC1 * 4 + X3) << OSS) + 2) / 4;
		X1 = ((int32_t)bmp085_coeff.AC3 * B6) >> 13;
		X2 = ((int32_t)bmp085_coeff.B1 * ((B6 * B6) >> 12)) >> 16;
		X3 = ((X1 + X2) + 2) >> 2;
		B4 = ((uint32_t)bmp085_coeff.AC4 * (uint32_t)(X3 + 32768UL)) >> 15;
        if (B4 == 0)
            return -2;
		B7 = ((uint32_t)UP - B3) * (50000UL >> OSS);
		if (B7 < 0x80000000UL)
			P = (B7 * 2) / B4;
		else
			P = (B7 / B4)  * 2;
		X1 = (P >> 8) * (P >> 8);
		X1 = (X1 * 3038L) >> 16;
		X2 = (-7357L * P) >> 16;
		P += (X1 + X2 + 3791L) >> 4;
		
		//DBG("Pressure = %d Pa, ", P);
		
		*p = P;
		return 0;
	}
	else
	{
		return -1;
	}
}

#include <math.h>

void pressure_convert(int32_t P, float *Pmm, float *Alt)
{
	// mm/Hg
	*Pmm = ((float)P * 75006.0f) / 10000000.0f;
	//DBG("Pressure = %3.0f mm, ", *Pmm);
	
	// Altitude calculate:
	*Alt = 44330.0f * (1 - powf(((float)P / 101325.0f), (1.0f / 5.255f)));
	//DBG("Altitude: %.2f, ", *Alt);
}

void pressure_convert_int(int32_t P, int32_t *Pmm, int32_t *Alt)
{
	uint64_t t;
	//*Pmm = ((float)P * 75006.0f) / 10000000.0f; // (mm/Hg)
	t = (uint64_t)P * (uint64_t)75006;
	*Pmm = t / 10000000;

	// Altitude calculate:
	//*Alt = 44330.0f * (1 - powf(((float)P / 101325.0f), (1.0f / 5.255f))); // (m)
	*Alt = -1; // didn't calculation
}
