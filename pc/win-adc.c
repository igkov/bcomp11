#include "dbg.h"
#include "adc.h"

void adc_init(void){}
int adc_get(int ch) {
	switch (ch) {
	case ADC_IN1:
		return 0;
	case ADC_IN2:
		return 703;
	case ADC_IN3:
		return 128;
	case ADC_IN4:
		return 512;
	default:
		DBG("adc_get(): unknown channel!\r\n");
		return -1;
	}
}
