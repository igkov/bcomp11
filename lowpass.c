#include <math.h>
#include "lowpass.h"

#define ABS(a) ((a)<0?-(a):(a))

float lowpass(float x, pfilter_t filter) {
	float delta = ABS(x - filter->xp);
	if (isnan(filter->xp)) {
		filter->xp = x;
	} else if (delta > filter->max_d) {
		filter->xp = x;
	} else {
		filter->xp = filter->a * x + (1 - filter->a) * filter->xp;
	}
	return filter->xp;
}
