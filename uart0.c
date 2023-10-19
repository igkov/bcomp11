#if !defined( WIN32 )
#include  "LPC11xx.h"
#endif
#include "uart0.h"
#include "dbg.h"

#if __UART_IRQ
void UART_IRQHandler(void) {
    uint8_t ch;
    uint8_t IIR_val = (LPC_UART->IIR >> 1) & 0x07;
    // Приемка:
    if (IIR_val == 0x03) {
        if (LPC_UART->LSR & 0x01) {
            ch = LPC_UART->RBR;
            // Вызываем обработчик пришедшего символа:
            UART_DISPATCHER(ch);
        } else {
            return;
        }
    } else 
    if (IIR_val == 0x02) {
        ch = LPC_UART->RBR;
        // Вызываем обработчик пришедшего символа:
        UART_DISPATCHER(ch);
    }
    return;
}
#endif

void uart0_init(uint32_t baudrate) {
    uint32_t  Fdiv;

#if __UART_IRQ
    NVIC_DisableIRQ(UART_IRQn);
#endif

    // UART power on:
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16) ;

    // UART I/O Config:
    LPC_IOCON->PIO1_6 &= ~0x07;
    LPC_IOCON->PIO1_6 |=  0x01;
    LPC_IOCON->PIO1_7 &= ~0x07;
    LPC_IOCON->PIO1_7 |=  0x01;

    // Enable UART clock:
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
    LPC_SYSCON->UARTCLKDIV     = 0x1;

    LPC_UART->LCR = 0x83; // 8 bits, no Parity, 1 Stop bit
    Fdiv = ((SystemCoreClock/LPC_SYSCON->UARTCLKDIV)/16)/baudrate;
    LPC_UART->DLM = Fdiv / 256;
    LPC_UART->DLL = Fdiv % 256;
    LPC_UART->LCR = 0x03;
#if 1
    LPC_UART->FCR = 0x07; // Enable and reset TX and RX FIFO.
#endif
#if __UART_IRQ
    LPC_UART->IER = 0x01 | 0x04; // IER_RBR | IER_RLS
    NVIC_EnableIRQ(UART_IRQn);
#endif
}

void uart0_putchar (char ch) {
   while (!(LPC_UART->LSR & 0x20));
   LPC_UART->THR  = ch;
}

uint8_t uart0_getchar (void) {
  while (!(LPC_UART->LSR & 0x01));
  return (LPC_UART->RBR);
}

void uart0_puts(const char * str) {
    while ((*str) != 0) {
        uart0_putchar(*str++);
    }
}
