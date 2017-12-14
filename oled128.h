#ifndef __OLED128_H__
#define __OLED128_H__

#include <stdint.h>

void oled_init(uint8_t contrast, uint8_t inverse);
void oled_contrast(uint8_t contrast);
void oled_draw(const uint8_t *bitmaparray);

#endif
