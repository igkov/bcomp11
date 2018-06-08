#include <math.h>
#include "adc.h"
#include "analog.h"

float analog_temp(const termist_t *par) {
	float R, T;
	int adc = adc_get(ADC_IN2);
	if ((adc<ADC_LIMIT)||
		(adc>(ADC_VALUE_MAX-ADC_LIMIT)))
		return NAN;  
	R = adc * BALANCE_R / (float)(ADC_VALUE_MAX+1-adc);
	//T  = 1.0f / (1.0f/TERMIST_T0 + (1.0f/TERMIST_B)*log(R/TERMIST_R0));
	T  = 1.0f / (1.0f/par->t0 + (1.0f/par->b)*log(R/par->r0));
	T -= 273.15f;
	return T;
}

/*
	Подробности здесь:
	http://igorkov.org/fuel-nmps
 */

#define FUEL_R1 100000.0f
#define FUEL_R2 33000.0f
#define FUEL_DIVIDER ((FUEL_R1+FUEL_R2)/(FUEL_R2))
#define FUEL_BASE 3.3f

#define FUEL_COEFF (-11.6f)
#define FUEL_START_LEVEL   ( 68.0f)
#define FUEL_START_VOLTAGE ( 2.0f)
#define CALIBRATE_FUEL(a) (FUEL_START_LEVEL+(a-FUEL_START_VOLTAGE)*FUEL_COEFF)

/* Функция возвращает посчитанное сопротивление ДУТ */
float analog_fuel(void) {
	float V;
	int adc = adc_get(ADC_IN3);
	if ((adc<ADC_LIMIT)||
		(adc>(ADC_VALUE_MAX-ADC_LIMIT)))
		return NAN;
	V = adc * FUEL_DIVIDER * FUEL_BASE / (ADC_VALUE_MAX+1);
	V = CALIBRATE_FUEL(V);
	return V;
}

/*
	Фунция получения напряжения питания через АЦП-вход.
	Для BCOMP11, не используется, здесь напряжение получается 
	только посредством запроса в ЭБУ.
 */

#if 1
float analog_volt(void) {
	return NAN;
}
#else
#define VOLT_R1 100000.0f
#define VOLT_R2 10000.0f
#define VOLT_DIVIDER ((VOLT_R1+VOLT_R2)/(VOLT_R2))
#define VOLT_BASE 3.3f
float analog_volt(void) {
	float V;
	int adc = adc_get(ADC_IN4);
	if ((adc<ADC_LIMIT)||
		(adc>(ADC_VALUE_MAX-ADC_LIMIT)))
		return NAN;
	V = adc * VOLT_DIVIDER * VOLT_BASE / (ADC_VALUE_MAX+1);
	return V;
}
#endif
