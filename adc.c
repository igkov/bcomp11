#include <LPC11xx.h>
#include "adc.h"
#include "dbg.h"

void adc_init(void) {
	// ADC power on:
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1UL <<  6) | (1UL << 16));
	// Configure PIN:
	LPC_IOCON->R_PIO0_11 = (2UL <<  0); // AD0
	LPC_IOCON->R_PIO1_0  = (2UL <<  0); // AD1
	LPC_IOCON->R_PIO1_2  = (2UL <<  0); // AD3
	LPC_IOCON->PIO1_4    = (1UL <<  0); // AD5
	LPC_IOCON->PIO1_10   = (1UL <<  0); // AD6
	// Configure directions:
	LPC_GPIO0->DIR &= ~((1UL<<11));
	LPC_GPIO1->DIR &= ~((1UL<<0)|(1UL<<2)|(1UL<<4));
	// configure ADC:
	LPC_SYSCON->PDRUNCFG      &= ~(1UL <<  4);  /* Enable power to ADC block  */
	LPC_SYSCON->SYSAHBCLKCTRL |=  (1UL << 13);  /* Enable clock to ADC block  */
}

int adc_get(int ch) {
	LPC_ADC->CR &= ~(7UL << 24);                  /* stop conversion          */
	switch (ch) {
	// X__ - P0.11 - ADC0
	case ADC_IN1: 
		LPC_ADC->CR        =  ( 1UL <<  0) |      /* select AD0 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;
	// X__ - P1.0 - ADC1
	case ADC_IN2: 
		LPC_ADC->CR        =  ( 1UL <<  1) |      /* select AD1 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;
	// X12 - P1.2 - ADC3
	case ADC_IN3: 
		LPC_ADC->CR        =  ( 1UL <<  3) |      /* select AD3 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;
	// X11 - P1.4 - ADC5
	case ADC_IN4: 
		LPC_ADC->CR        =  ( 1UL <<  5) |      /* select AD5 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;
	// X18 - P1.10 - ADC6
	case ADC_IN5: 
		LPC_ADC->CR        =  ( 1UL <<  6) |      /* select AD5 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;

	default:
		DBG("adc_get(): unknown channel!\r\n");
		return -1;
	}
	LPC_ADC->CR |=  (1UL << 24);                  /* Start A/D Conversion      */
	while (!(LPC_ADC->GDR & (1UL << 31)));       /* Wait for Conversion end   */
	return (LPC_ADC->GDR >> 6) & ADC_VALUE_MAX;  /* Store converted value     */
}
