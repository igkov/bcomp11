#include <LPC11xx.h>
#include "timer.h"

void timer0_init(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL<<9);
	// timer init:
	LPC_TMR32B0->TC  = 0;
	LPC_TMR32B0->PR  = 48; // 1MHz ticks freq
	LPC_TMR32B0->TCR = 0x01;
}

uint32_t timer0_get(void) {
	return LPC_TMR32B0->TC;
}

void delay_mks(uint32_t mks) {
	uint32_t mksStart = LPC_TMR32B0->TC;
	while ((LPC_TMR32B0->TC-mksStart) < mks);
}

void timer1_init(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL<<6)|(1UL<<10)|(1UL<<16);
	// pio init:
	LPC_IOCON->R_PIO1_1 &= ~0x07; /*  As  */
	LPC_IOCON->R_PIO1_1 |=  0x01; /* GPIO */
  	// set 0:
	LPC_GPIO1->DIR  |=  (1UL<<1);
	LPC_GPIO1->DATA &= ~(1UL<<1);
	// timer init:
	LPC_TMR32B1->TC  = 0;
	LPC_TMR32B1->PR  = 0;
	LPC_TMR32B1->TCR = 0x01; // start
}

void timer1_pwm_on(void) {
	LPC_IOCON->R_PIO1_1 &= ~0x07;
	LPC_IOCON->R_PIO1_1 |=  0x03;
	// start clk:
	LPC_TMR32B1->MCR = 0x02<<0; // Reset on MR0
	LPC_TMR32B1->EMR = (1<<0)|(3<<4);
}

void timer1_pwm_off(void) {
	LPC_IOCON->R_PIO1_1 &= ~0x07;
	LPC_IOCON->R_PIO1_1 |=  0x01;
}

void timer1_pwm_freq(int freq) {
	LPC_IOCON->R_PIO1_1 &= ~0x07;
	LPC_IOCON->R_PIO1_1 |=  0x03;
	LPC_TMR32B1->MR0 = 48000000/2/freq;
	LPC_TMR32B1->TC  = 0;
}

uint32_t timer1_get(void) {
	return LPC_TMR32B1->TC;
}


