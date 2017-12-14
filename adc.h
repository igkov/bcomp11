#ifndef __ADC_H__
#define __ADC_H__

#define ADC_VALUE_MAX 0x3FF
#define ADC_LIMIT     0x20

// X12 - P1.2 - AD3
#define ADC_CH1 0
// X11 - P1.4 - AD5
#define ADC_CH2 1
// ...
#define ADC_CH3 2

extern void adc_init(void);
extern int adc_get(int ch);

#endif
