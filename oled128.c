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
#if ( OLED_SSD1306_SUPPORT == 1 )
	if (bconfig.oled_type == 0) {
		ssd1306_init(contrast, inverse);
	} else 
#endif
#if ( OLED_SH1106_SUPPORT == 1 )
	if (bconfig.oled_type == 1) {
		// Задержка перед инициализацией контроллера SH1106.
		// Призвана предотвратить ошибки инициализации экрана
		// (замечены ситуации на Release-сборке, когда часть 
		// экрана не отображает информацию или содержит мусор).
		delay_ms(100);
		sh1106_init(contrast, inverse);
	} else
#endif
	{
		// NOP
	}
}

void oled_draw(const uint8_t *bitmaparray) {
#if ( OLED_SSD1306_SUPPORT == 1 )
	if (bconfig.oled_type == 0) {
		ssd1306_draw(bitmaparray);
	} else 
#endif
#if ( OLED_SH1106_SUPPORT == 1 )
	if (bconfig.oled_type == 1) {
		sh1106_draw(bitmaparray);
	} else
#endif
	{
		// NOP
	}
}

void oled_contrast(uint8_t contrast) {
#if ( OLED_SSD1306_SUPPORT == 1 )
	if (bconfig.oled_type == 0) {
		ssd1306_contrast(contrast);
	} else 
#endif
#if ( OLED_SH1106_SUPPORT == 1 )
	if (bconfig.oled_type == 1) {
		sh1106_contrast(contrast);
	} else
#endif
	{
		// NOP
	}
}
