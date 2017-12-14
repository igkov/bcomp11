#include <stdio.h>
#include <stdlib.h>
#include "eeprom.h"

static FILE *ee_file;
// (1<<16) 512kbit = 64kb
// (1<<12) 32kbit = 4kb = 128 value 
// (1<<11) 16kbit = 2kb = 64 value
//

#define EE_FILESIZE (1<<11)

unsigned char ee_write(unsigned short addr, unsigned char *buff, unsigned char len) {
	if (addr+len > EE_FILESIZE) {
		len = EE_FILESIZE-addr;
	}
	fseek(ee_file, addr, SEEK_SET);
	fwrite(buff, 1, len, ee_file);
	return 0;
}

unsigned char ee_clr(unsigned short addr, unsigned char len) {
	int i;
	unsigned char data = 0xFF;
	if (addr+len > EE_FILESIZE) {
		len = EE_FILESIZE-addr;
	}
	fseek(ee_file, addr, SEEK_SET);
	for (i=0; i<len; i++) {
		fwrite(&data, 1, 1, ee_file);
	}
	return 0;
}

unsigned char ee_read(unsigned short addr, unsigned char *buff, unsigned char len) {
	if (addr+len > EE_FILESIZE) {
		len = EE_FILESIZE-addr;
	}
	fseek(ee_file, addr, SEEK_SET);
	fread(buff, 1, len, ee_file);
	return 0;
}

void ee_init(void) {
	unsigned char data = 0;
	ee_file = fopen("eeprom.bin","rb");
	if (ee_file == NULL) {
		ee_file = fopen("eeprom.bin","wb");
		fseek(ee_file, EE_FILESIZE-1, SEEK_SET);
		fwrite(&data, 1, 1, ee_file);
		fseek(ee_file, 0, SEEK_SET);
	} else {
		fclose(ee_file);
		ee_file = fopen("eeprom.bin","rb+");
		if (ee_file == NULL) {
			// error
			printf("Cannot open file eeprom.bin!\r\n");
			return;
		}
	}
	return;
}
