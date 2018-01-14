#ifndef __UART0_H__
#define __UART0_H__

#include <stdint.h>

/* Используем прерывания для приема данных? */
#define __UART_IRQ   0
extern void nmea_proc(uint8_t ch);
#define UART_DISPATCHER(a) nmea_proc(a)

void    uart0_init(uint32_t baudrate);
void    uart0_putchar(uint8_t ch);
uint8_t uart0_getchar(void);
int     uart0_isdata(void);
void    uart0_puts(const uint8_t *str);

#endif
