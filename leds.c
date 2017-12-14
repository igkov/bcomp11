#include <stdint.h>
#include <LPC11xx.H>
#include "leds.h"

void leds_init(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL <<  6);
	LPC_GPIO2->DIR  |=  (1UL<<9)|(1UL<<10);
	LPC_GPIO2->DATA &= ~((1UL<<9)|(1UL<<10));
}

void led_red(int state) {
	if (state) {
		LPC_GPIO2->DATA |=  (1UL<<9);
	} else {
		LPC_GPIO2->DATA &= ~(1UL<<9);
	}
}

void led_green(int state) {
	if (state) {
		LPC_GPIO2->DATA |=  (1UL<<10);
	} else {
		LPC_GPIO2->DATA &= ~(1UL<<10);
	}
}
