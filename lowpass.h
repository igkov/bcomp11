#ifndef __LOWPASS_H__
#define __LOWPASS_H__

#include <stdint.h>

#define LOWPASS_A  0.03
#define CALIBRATE_LOWPASS_A 0.09

typedef struct
{
	float a;
	float xp;
	float max_d;
} filter_t, *pfilter_t;

float lowpass(float x, pfilter_t filter);

#endif //__LOWPASS_H__
