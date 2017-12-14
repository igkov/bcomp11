#include <stdint.h>
#include <string.h>
#include "errors.h"

static char to_char(int value) {
	value &= 0x0F;
	if (value < 10) {
		return '0'+value;
	} else {
		return 'A'+value;
	}
}

void error_decrypt(uint16_t code, char *err) {
	switch ((code&0xC000)>>14) {
	case 0: err[0] = 'P'; break;
	case 1: err[0] = 'C'; break;
	case 2: err[0] = 'B'; break;
	case 3: err[0] = 'U'; break;
	}
	switch ((code&0x3000)>>12) {
	case 0: err[1] = '0'; break;
	case 1: err[1] = '1'; break;
	case 2: err[1] = '2'; break;
	case 3: err[1] = '3'; break;
	}
	err[2] = to_char((code & 0x0F00)>>8);
	err[3] = to_char((code & 0x00F0)>>4);
	err[4] = to_char((code & 0x000F)>>0);
	err[5] = 0;
}
