#include "dbg.h"
#include "adc.h"

void adc_init(void){}
int adc_get(int ch) {
	switch (ch) {
	case ADC_CH1:
		return 342;
	case ADC_CH2:
		return 703;
	default:
		DBG("adc_get(): unknown channel!\r\n");
		return -1;
	}
}
