#ifndef __ADC_H__
#define __ADC_H__

#define ADC_VALUE_MAX 0x3FF
#define ADC_LIMIT     0x20

// -------------------------------------------------------------------------
// Новая нумерация, в соответствии с HW 2.0: github.com/igkov/bcomp11hw.
// AN1 (ADC0), AN2 (ADC1), AN3 (ADC3), AN4 (ADC5), AN5 (ADC6):
// -------------------------------------------------------------------------

// ANALOG BUTTONS (OPTIONAL)
#define ADC_IN1 1
// TERMISTOR (OPTIONAL)
#define ADC_IN2 2
// FUEL (OPTIONAL, pad X12)
#define ADC_IN3 3
// EXTERNAL VOLTAGE (OPTIONAL, pad X11)
#define ADC_IN4 4
// RFU (OPTIONAL, pad X18)
#define ADC_IN5 5

//
// --------------------------------------------------------------------------
//


extern void adc_init(void);
extern int adc_get(int ch);

#endif
