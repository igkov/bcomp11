#include <LPC11xx.h>
#include "adc.h"
#include "dbg.h"

void adc_init(void) {
	// ADC power on:
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1UL <<  6) | (1UL << 16));
	// Configure PIN GPIO0.11 for AD3/AD5:
	LPC_IOCON->R_PIO1_2 = (2UL <<  0);
	LPC_IOCON->PIO1_4   = (1UL <<  0);
	//
	// TODO!!!
	LPC_GPIO1->DIR &= ~((1UL<<2)|(1UL<<4));
	// configure ADC:
	LPC_SYSCON->PDRUNCFG      &= ~(1UL <<  4);  /* Enable power to ADC block  */
	LPC_SYSCON->SYSAHBCLKCTRL |=  (1UL << 13);  /* Enable clock to ADC block  */
}

int adc_get(int ch) {
	LPC_ADC->CR &= ~(7UL << 24);                  /* stop conversion          */
	switch (ch) {
	// X12 - P1.2 - AD3
	case ADC_CH1: 
		LPC_ADC->CR        =  ( 1UL <<  3) |      /* select AD3 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;
	// X11 - P1.4 - AD5
	case ADC_CH2: 
		LPC_ADC->CR        =  ( 1UL <<  5) |      /* select AD5 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;
	// -   - P0.11 - AD0
	case ADC_CH3: 
		LPC_ADC->CR        =  ( 1UL <<  0) |      /* select AD0 pin           */
		                      (23UL <<  8) |      /* ADC clock is 24MHz/24    */
		                      ( 1UL << 21);       /* enable ADC               */
		break;
	// -   - P1.0 - AD1
	case ADC_CH4: 
		LPC_ADC->CR        =  ( 1UL <<  1) |      /* select AD1 pin           */
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
