#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <stdint.h>
#include "icons.h"

#define SIZE_X 128
#define SIZE_Y 64

// Обновление экрана:
void graph_update(void);
void graph_clear(void);

// Простые функции вывода данных (битовые несжатые шрифты):
void graph_puts8(int x, int y, int w, const char *str);
void graph_puts16(int x, int y, int w, const char *str);
void graph_ico16(int x, int y, const uint8_t *ico, int xsize);
void graph_ico16(int x, int y, const uint8_t *ico, int xsize);

// Работа с полноценными шрифтами:
void graph_puts32c(int ox, int oy, char *str);
// Новая функция вывода иконок:
void graph_pic(const ico_t *p, int ox, int oy);

// Графика:
void graph_line(int x1, int y1, int x2, int y2);

#endif
