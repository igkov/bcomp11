#include <lpc11xx.h>
#include "dbg.h"
#include "event.h"
#include "timer.h"
#include "mbus.h"
#include "leds.h"

/*
	Sniffer mode = 1

	Режим сниффера, в данном режиме логика работает на приём байт с шины.
	Требуется для анализа протокола и съема кодов.

	Посылает следующие кодовые байты на шину:
	1. FF - принятый байтю
	2. S - активный BUSY (=0).
	3. U - неактивный BUSY (=1).
 */
#define SNIFFER_MODE 1

#define PIN_MBUSY_OUT    (LPC_GPIO2->DIR  |=  (1UL<<0))
#define PIN_MBUSY_IN     (LPC_GPIO2->DIR  &= ~(1UL<<0))
#define PIN_MBUSY_SET0   (LPC_GPIO2->DATA &= ~(1UL<<0))
#define PIN_MBUSY_SET1   (LPC_GPIO2->DATA |=  (1UL<<0))
#define PIN_MBUSY_GET    (LPC_GPIO2->DATA &   (1UL<<0))

#define PIN_MCLOCK_OUT   (LPC_GPIO2->DIR  |=  (1UL<<1))
#define PIN_MCLOCK_IN    (LPC_GPIO2->DIR  &= ~(1UL<<1))
#define PIN_MCLOCK_SET0  (LPC_GPIO2->DATA &= ~(1UL<<1))
#define PIN_MCLOCK_SET1  (LPC_GPIO2->DATA |=  (1UL<<1))
#define PIN_MCLOCK_GET   (LPC_GPIO2->DATA &   (1UL<<1))

#define PIN_MDATA_OUT    (LPC_GPIO2->DIR  |=  (1UL<<2))
#define PIN_MDATA_IN     (LPC_GPIO2->DIR  &= ~(1UL<<2))
#define PIN_MDATA_SET0   (LPC_GPIO2->DATA &= ~(1UL<<2))
#define PIN_MDATA_SET1   (LPC_GPIO2->DATA |=  (1UL<<2))
#define PIN_MDATA_GET    (LPC_GPIO2->DATA &   (1UL<<2))

void mbus_init(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1UL <<  6);

	// Init M-BUSY, M_CLOCK, M-DATA in and pull-up.
	PIN_MBUSY_OUT;
	PIN_MCLOCK_OUT;
	PIN_MDATA_OUT;
	
	// ----
	// Init M-CLOCK interrupt:
	// ----
	// Init ISENSE:
	// 0 = Interrupt on pin PIOn_x is configured as edge sensitive.
	// 1 = Interrupt on pin PIOn_x is configured as level sensitive. 
	LPC_GPIO2->IS &= ~(1UL<<1);
	// Init IBE:
	// 0 = Interrupt on pin PIOn_x is controlled through register GPIOnIEV.
	// 1 = Both edges on pin PIOn_x trigger an interrupt.
	LPC_GPIO2->IBE |=  (1UL<<0);
	LPC_GPIO2->IBE &= ~(1UL<<1);
	// Init IEV:
	// 0 = Depending on setting in register GPIOnIS (see IS), falling edges or LOW level on pin PIOn_x trigger an interrupt.
	// 1 = Depending on setting in register GPIOnIS (see IS), rising edges or HIGH level on pin PIOn_x trigger an interrupt.
	LPC_GPIO2->IEV |= (1UL<<1);
	// Init IE:
	// 0 = Interrupt on pin PIOn_x is masked.
	// 1 = Interrupt on pin PIOn_x is not masked.
	LPC_GPIO2->IE |= (1UL<<0)|(1UL<<1);
	
	NVIC_EnableIRQ(EINT2_IRQn);
	
#if (SNIFFER_MODE == 1)
	while (1) {
		__WFI();
	}
#endif
}

uint8_t bdata = 0;
uint8_t boff = 0;

//This is a function that sends a byte to the HU - (not using interrupts)
void mbus_sendb(uint8_t b){
	int n;

	NVIC_DisableIRQ(EINT2_IRQn); 
	PIN_MDATA_OUT;
  	for(n = 7; n >= 0; n--) {
    	if(b & (1<<n)) {
      		PIN_MDATA_SET1;
    	} else {
      		PIN_MDATA_SET0;
    	}  
    	while (PIN_MCLOCK_GET == 1);
    	while (PIN_MCLOCK_GET == 0);
  	} 
  	PIN_MDATA_IN;
  	NVIC_EnableIRQ(EINT2_IRQn);
}

void PIOINT2_IRQHandler(void) {
	uint32_t status = LPC_GPIO2->MIS;
	if (status & (1UL<<0)) {
		DBG("irq: pin busy = %d!\r\n", PIN_MBUSY_GET);

		if (PIN_MBUSY_GET) {
			led_green(0);
#if (SNIFFER_MODE == 1)
			LPC_UART->THR  = 'U';
			while (!(LPC_UART->LSR & 0x20));
#endif
		} else {
			led_green(1);
#if (SNIFFER_MODE == 1)
			LPC_UART->THR  = 'S';
			while (!(LPC_UART->LSR & 0x20));
#endif
			// При установке BUSY-сигнала, начинаем приемку нового байта.
			boff = 0;
		}
	}
	if (status & (1UL<<1)) {
		DBG("irq: pin clk rise, pin data = %d!\r\n", PIN_MDATA_GET);

		if (boff == 0) {
			bdata = 0;
		}
		if (PIN_MDATA_GET) {
			bdata |= (1<<boff);
		}
		boff++;
		if (boff == 8) {
			boff = 0;

#if (SNIFFER_MODE == 1)
			if ((bdata&0xF0) >= 0xA0) {
				LPC_UART->THR  = 'A'+((bdata&0xF0)>>4)-10;
			} else {
				LPC_UART->THR  = '0'+((bdata&0xF0)>>4);
			}
			while (!(LPC_UART->LSR & 0x20));
			if ((bdata&0x0F) >= 0x0A) {
				LPC_UART->THR  = 'A'+(bdata&0x0F)-10;
			} else {
				LPC_UART->THR  = '0'+(bdata&0x0F);
			}
#endif
		}
	}
	LPC_GPIO2->IC = status;
}

//Notify HU that we want to trigger the first initiate procedure to add a new device (CD-CHGR) by pulling BUSY line low for 1s
void mbus_init_CDCHRG() {
	NVIC_DisableIRQ(EINT2_IRQn); 

	while (PIN_MBUSY_GET == 0); 
	delay_mks(10);
  
	PIN_MBUSY_OUT;
	PIN_MBUSY_SET0;
	delay_ms(1200);
	PIN_MBUSY_SET1;
	PIN_MBUSY_IN;

	NVIC_EnableIRQ(EINT2_IRQn); 
}

