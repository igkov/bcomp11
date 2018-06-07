#ifndef __ADC_H__
#define __ADC_H__

#define ADC_VALUE_MAX 0x3FF
#define ADC_LIMIT     0x20

//
// Старая нумерация каналов из HW 1.0:
//


// X12 - P1.2 - AD3
#define ADC_CH1 0
// X11 - P1.4 - AD5
#define ADC_CH2 1
// -   - P0.11 - AD0
#define ADC_CH3 2
// -   - P1.0 - AD1
#define ADC_CH4 3

//
// =========================================
//

//
// Новая нумерация, в соответствии с HW 2.0: github.com/igkov/bcomp11hw.
// AN1 (ADC0), AN2 (ADC1), AN3 (ADC3):
//

// TERMISTOR (OPTIONAL)
#define ADC_IN1 2
// ANALOG BUTTONS (OPTIONAL)
#define ADC_IN2 3
// FUEL (OPTIONAL)
#define ADC_IN3 0

//
// =========================================
//


extern void adc_init(void);
extern int adc_get(int ch);

#endif
