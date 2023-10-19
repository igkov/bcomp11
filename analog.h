#ifndef __ANALOG_H__
#define __ANALOG_H__

#if defined( RELEASE )
// Mitsubishi Thermistor:
#define TERMIST_B  3700.0f
#define TERMIST_T0 313.15f
#define TERMIST_R0 1000.0f
#else
// EPCOS k164 10k Thermistor:
#define TERMIST_B  4300.0f
#define TERMIST_T0 298.15f
#define TERMIST_R0 10000.0f
#endif
#define BALANCE_R  10000.0f

typedef struct {
	float b;
	float t0;
	float r0;
} termist_t;

float analog_temp(const termist_t *par); // Получение температуры, пересчет по данным в структуре.
float analog_fuel(void); // Получение уровня топлива (пока просто напряжение на датчике).
float analog_volt(void); // Функция получения напряжения бортовой сети (через АЦП).

#endif
