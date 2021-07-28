#ifndef __BMP085__
#define __BMP085__

#include <stdint.h>

int bmp085_init(void);

int bmp085_start_baro(int prec);
int bmp085_start_temp(void);

int bmp085_get_temp(int32_t *t);
int bmp085_get_baro(int32_t *p);

void pressure_convert(int32_t P, float *Pmm, float *Alt);
void pressure_convert_int(int32_t P, int32_t *Pmm, int32_t *Alt);

#endif // __BMP085__
