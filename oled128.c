#include "oled128.h"
#include "bcomp.h"
#include "event.h"

extern void sh1106_init(uint8_t contrast, uint8_t inverse);
extern void sh1106_draw(const uint8_t *bitmaparray);
extern void sh1106_contrast(uint8_t contrast);
extern void ssd1306_init(uint8_t contrast, uint8_t inverse);
extern void ssd1306_draw(const uint8_t *bitmaparray);
extern void ssd1306_contrast(uint8_t contrast);

void oled_init(uint8_t contrast, uint8_t inverse) {
	if (bconfig.oled_type == 0) {
		ssd1306_init(contrast, inverse);
	} else 
	if (bconfig.oled_type == 1) {
		// Задержка перед инициализацией контроллера SH1106.
		// Призвана предотвратить ошибки инициализации экрана
		// (замечены ситуации на Release-сборке, когда часть 
		// экрана не отображает информацию или содержит мусор).
		delay_ms(100);
		sh1106_init(contrast, inverse);
	}
}

void oled_draw(const uint8_t *bitmaparray) {
	if (bconfig.oled_type == 0) {
		ssd1306_draw(bitmaparray);
	} else 
	if (bconfig.oled_type == 1) {
		sh1106_draw(bitmaparray);
	}
}

void oled_contrast(uint8_t contrast) {
	if (bconfig.oled_type == 0) {
		ssd1306_contrast(contrast);
	} else 
	if (bconfig.oled_type == 1) {
		sh1106_contrast(contrast);
	}
}
