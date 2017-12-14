#include <stdio.h>
#include "uart0.h"

void uart0_init(uint32_t speed) {
	// NOP
}

uint8_t uart0_getchar(void) {
	return getch();
}

void uart0_putchar(const uint8_t c) {
	putchar(c);
}

void uart0_puts(const uint8_t *str) {
	int i = 0;
	while (str[i]) {
		putchar(str[i]);
		i++;
	}
}