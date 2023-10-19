/*
	OLED 128x64 1.3" driver.
	SH1106
 */
#include <stdio.h>
#include <string.h>

#include "i2c.h"
#include "dbg.h"
#include "event.h"

#define OLED_ADDRESS					0x78 
//#define OLED_ADDRESS					0x7A 

#define OzOLED_COMMAND_MODE				0x80
#define OzOLED_DATA_MODE				0x40

static int sendCommand(uint8_t command) {
	return i2c_write(OLED_ADDRESS, OzOLED_COMMAND_MODE, &command, 1);
}

#if 0
static int sendCommandN(uint8_t *command, int size) {
	return i2c_write(OLED_ADDRESS, OzOLED_COMMAND_MODE, command, size);
}
#endif

#if 0
static int sendData(uint8_t data) {
	return i2c_write(OLED_ADDRESS, OzOLED_DATA_MODE, &data, 1);
}
#endif

static int sendDataN(uint8_t *data, int size) {
	return i2c_write(OLED_ADDRESS, OzOLED_DATA_MODE, data, size);
}

void sh1106_draw(const uint8_t *bitmaparray) {
	sendCommand(0x02);
	sendCommand(0x10);
	sendCommand(0xB0);
	sendDataN((uint8_t*)&bitmaparray[128*0], 128+2);
	sendCommand(0xB1);
	sendDataN((uint8_t*)&bitmaparray[128*1-2], 128+4);
	sendCommand(0xB2);
	sendDataN((uint8_t*)&bitmaparray[128*2-2], 128+4);
	sendCommand(0xB3);
	sendDataN((uint8_t*)&bitmaparray[128*3-2], 128+4);
	sendCommand(0xB4);
	sendDataN((uint8_t*)&bitmaparray[128*4-2], 128+4);
	sendCommand(0xB5);
	sendDataN((uint8_t*)&bitmaparray[128*5-2], 128+4);
	sendCommand(0xB6);
	sendDataN((uint8_t*)&bitmaparray[128*6-2], 128+4);
	sendCommand(0xB7);
	sendDataN((uint8_t*)&bitmaparray[128*7-2], 128+4);
}

void sh1106_contrast(uint8_t contrast) {
	sendCommand(0x81); sendCommand(contrast); // 0x81,0x7f — Set Contrast Control (0x7f) — яркость. 
}

void sh1106_init(uint8_t contrast, uint8_t inverse) {
	sendCommand(0xAE); 
	sendCommand(0xA8); sendCommand(0x3F); // 0xA8,0x3F — Set Multiplex Ratio (0x3F) — эта команда определяет, сколько строк будет использовано.
	sendCommand(0xD5);
	sendCommand(0x81); sendCommand(contrast); // 0x81,0x7f — Set Contrast Control (0x7f) — яркость. 
	sendCommand(0xA0); // 0xA1 — Set Segment Re-map — развертка по горизонтали слева направо.
	sendCommand(0xC0); // 0xC0 — Set Common Output Scan Direction — развертка сверху вниз
	sendCommand(0xDA); sendCommand(0x12); // 0xDA,0x12 — Common Pads Hardware Configuration (Alternative Mode Set) — прогрессивная развертка. Можно включить и чересстрочную.
	sendCommand(0xD3); sendCommand(0x00); // 0xD3,0x00 — Set Display Offset (0x00)
	sendCommand(0x40); // 0x40 — Set Display Start Line 0
	if (inverse) {
		sendCommand(0xA7);
	} else {
		sendCommand(0xA6);
	}
	sendCommand(0xAF); // 0xAF — Display ON
}
