#include <stdio.h>
#include <string.h>
#include "uart2.h"

static FILE *log_file=NULL;

void uart2_init(int speed) {
	unsigned char filename[32]; 
	int t = time(NULL);
	sprintf(filename, "log_%d.txt", t);
	log_file = fopen(filename,"wb");
	if (log_file == NULL) {
		// error
	}
}

int uart2_getchar(void) {
	// TODO
	return 0;
}

int uart2_putchar(int c) {
	unsigned char data = c;
	if (log_file) {
		fwrite(&data, 1, 1, log_file);
	}
}

int uart2_puts(const char *str) {
	if (log_file) {
		fwrite(str, 1, strlen(str), log_file);
	}
}