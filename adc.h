#ifndef __ADC_H__
#define __ADC_H__

#define ADC_VALUE_MAX 0x3FF
#define ADC_LIMIT     0x20

// -------------------------------------------------------------------------
// Новая нумерация, в соответствии с HW 2.0: github.com/igkov/bcomp11hw.
// AN1 (ADC0), AN2 (ADC1), AN3 (ADC3):
// -------------------------------------------------------------------------

// ANALOG BUTTONS (OPTIONAL)
#define ADC_IN1 2
// TERMISTOR (OPTIONAL)
#define ADC_IN2 1
// FUEL (OPTIONAL)
#define ADC_IN3 3
// RFU (OPTIONAL, pad X11)
#define ADC_IN4 4

//
// --------------------------------------------------------------------------
//


extern void adc_init(void);
extern int adc_get(int ch);

#endif
