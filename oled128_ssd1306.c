/*
	OLED 128x64 0.96" driver.
	SSD1306
 */
#include <stdio.h>
#include <string.h>

#include "i2c.h"
#include "event.h"

#define OLED_ADDRESS					0x78 

// registers
#define OzOLED_COMMAND_MODE				0x80
#define OzOLED_DATA_MODE				0x40

// commands
#define OzOLED_CMD_DISPLAY_OFF			0xAE
#define OzOLED_CMD_DISPLAY_ON			0xAF
#define OzOLED_CMD_NORMAL_DISPLAY		0xA6
#define OzOLED_CMD_INVERSE_DISPLAY		0xA7
#define OzOLED_CMD_SET_BRIGHTNESS		0x81

static int sendCommand(uint8_t command) {
	return i2c_write(OLED_ADDRESS, OzOLED_COMMAND_MODE, &command, 1);
}

static int sendDataN(uint8_t *data, int size) {
	return i2c_write(OLED_ADDRESS, OzOLED_DATA_MODE, data, size);
}

static void oled_setPowerOff(void) {
	sendCommand(OzOLED_CMD_DISPLAY_OFF);
}

static void oled_setPowerOn(void) {
	sendCommand(OzOLED_CMD_DISPLAY_ON);
}

static void oled_setCursorXY(uint8_t X, uint8_t Y) {
	// Y - 1 unit = 1 page (8 pixel rows)
	// X - 1 unit = 8 pixel columns
	sendCommand(0x00 + ((8*X)    & 0x0F)); 	// set column lower address
	sendCommand(0x10 + ((8*X>>4) & 0x0F)); 	// set column higher address
	sendCommand(0xB0 + Y);                  // set page address
}

#if 1
// safe draw
void ssd1306_draw(const uint8_t *bitmaparray) {
	int i;
	for (i = 0; i < 8; i++) {
		oled_setCursorXY( 0, i );
		sendDataN((uint8_t*)&bitmaparray[i*128], 128);
	}
}
#else
// this function realization have line-addresation problems in some screens
void ssd1306_draw(const uint8_t *bitmaparray) {
	oled_setCursorXY( 0, 0 );
	sendDataN((uint8_t*)bitmaparray, 128*64/8);
}
#endif

// SSD1306 Commandset
// ------------------
// Fundamental Commands
#define ASA_DISPLAY_ALL_ON_RESUME       0xA4
// Addressing Setting Commands
#define ASA_MEMORY_ADDR_MODE            0x20
// Hardware Configuration Commands
#define ASA_SET_START_LINE              0x40
#define ASA_SET_SEGMENT_REMAP           0xA0
#define ASA_SET_MULTIPLEX_RATIO         0xA8
#define ASA_COM_SCAN_DIR_DEC            0xC8
#define ASA_SET_DISPLAY_OFFSET          0xD3
#define ASA_SET_COM_PINS                0xDA
#define ASA_CHARGE_PUMP                 0x8D
// Timing & Driving Scheme Setting Commands
#define ASA_SET_DISPLAY_CLOCK_DIV_RATIO 0xD5
#define ASA_SET_PRECHARGE_PERIOD        0xD9
#define ASA_SET_VCOM_DESELECT           0xDB 

void ssd1306_contrast(uint8_t contrast) {
	//oled_setPowerOff();
	sendCommand(OzOLED_CMD_SET_BRIGHTNESS);
	sendCommand(contrast);
	//oled_setPowerOn();
}

void ssd1306_init(uint8_t contrast, uint8_t inverse) {
	oled_setPowerOff();
	sendCommand(ASA_SET_DISPLAY_CLOCK_DIV_RATIO);
	sendCommand(0x80);
	sendCommand(ASA_SET_MULTIPLEX_RATIO);
	sendCommand(0x3F);
	sendCommand(ASA_SET_DISPLAY_OFFSET);
	sendCommand(0x0);
	sendCommand(ASA_SET_START_LINE | 0x0);
	//set charge pump disable=0x10, enable=0x14   reset default = 0x10  Causes alignment problem with big display  Needed for little display!! 
	sendCommand(ASA_CHARGE_PUMP);
#if 1
	sendCommand(0x14);  // enable
#else
	sendCommand(0x10);  // disable
#endif
	sendCommand(ASA_MEMORY_ADDR_MODE);
	sendCommand(0x00);
	sendCommand(ASA_SET_SEGMENT_REMAP | 0x1);
	sendCommand(ASA_COM_SCAN_DIR_DEC);
	sendCommand(ASA_SET_COM_PINS);
	sendCommand(0x12);
	sendCommand(OzOLED_CMD_SET_BRIGHTNESS);
	sendCommand(contrast);
	sendCommand(ASA_SET_PRECHARGE_PERIOD);
	sendCommand(0xF1);
	sendCommand(ASA_SET_VCOM_DESELECT);
	sendCommand(0x40);
	sendCommand(ASA_DISPLAY_ALL_ON_RESUME);
	if (inverse) {
		sendCommand(OzOLED_CMD_INVERSE_DISPLAY);
	} else {
		sendCommand(OzOLED_CMD_NORMAL_DISPLAY);
	}
	oled_setPowerOn();
}
