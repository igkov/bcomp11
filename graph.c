#include <stdio.h>
#include <string.h>

#include "dbg.h"
#include "graph.h"
#include "icons.h"
#include "lzss.h"

#if defined( WIN32 )
#include "winlcd.h"
#else
#include "oled128.h"
#endif

// Extern:
extern const uint8_t font6x8[256][6];
extern const uint8_t font12x16[256][24];
extern int _sprintf(char *out, const char *format, ...);

// Framebuffer:
// 1 bit per pixel:
static uint8_t frame[128*64/8];

// Низкоуровневая работа с пикселями экрана:
#define SET_PIXEL(x,y) frame[SIZE_X * ((y)/8) + (x)] |= 1<<(y&0x07);
#define CLR_PIXEL(x,y) frame[SIZE_X * ((y)/8) + (x)] &= ~(1<<(y&0x07));

#if !defined( BCOMP_V2 )
// Font 6x8
void graph_puts8(int x, int y, int w, const char *str) {
	int i = 0;
	int fx = 0;
	if (w) {
		x = x - 6*strlen(str)/2;
		if (x < 0) {
			x = 0;
		}
	}
	while (str[i] != 0) {
		for (fx=0; fx < 6; fx++) {
			int offset;
			if (x+fx > 255) {
				continue;
			}
			offset = SIZE_X * (y/8) + (x + fx);
			frame[offset] = font6x8[str[i]][fx];
		}
		i++;
		x += fx;
	}
}
#endif

// Font 12x16
void graph_puts16(int x, int y, int w, const char *str) {
	int i = 0;
	int fx = 0;
	int fy = y>>3;
	int fm = y&0x07;
	uint8_t ch;
	if (w) {
		x = x - 12*strlen(str)/2;
		if (x < 0) {
			x = 0;
		}
	}
	while ((ch = str[i]) != 0) {
		// nop codes 0x00-0x1F
		// nop codes 0x80-0xAF
		if (ch < 0x20) {
			continue;
		} else
		if (ch >= 0xB0) {
			ch -= (0x20 + 0x030);
		} else {
			ch -= 0x20;
		}
		for (fx=0; fx < 12; fx++) {
			frame[SIZE_X * (fy + 0) + (x + fx)] |= font12x16[ch][fx]<<fm;
			frame[SIZE_X * (fy + 1) + (x + fx)] |= font12x16[ch][fx]>>(8-fm);
			frame[SIZE_X * (fy + 1) + (x + fx)] |= font12x16[ch][fx+12]<<fm;
			frame[SIZE_X * (fy + 2) + (x + fx)] |= font12x16[ch][fx+12]>>(8-fm);
		}
		i++;
		x += fx;
	}
}

#if !defined( BCOMP_V2 )
// ICO XXx16
void graph_ico16(int x, int y, const uint8_t *ico, int size) {
	int fx;
	int fy = y>>3;
	int fm = y&0x07;
	for (fx=0; fx < size; fx++) {
		frame[SIZE_X * (fy + 0) + (x + fx)] |= ico[fx+0*size]<<fm;
		frame[SIZE_X * (fy + 1) + (x + fx)] |= ico[fx+0*size]>>(8-fm);
		frame[SIZE_X * (fy + 1) + (x + fx)] |= ico[fx+1*size]<<fm;
		frame[SIZE_X * (fy + 2) + (x + fx)] |= ico[fx+1*size]>>(8-fm);
	}
}
#endif

#if !defined( __GCC__ )
// Вывод графической иконки по дескриптору.
// Поддерживает сжатие тела иконки.
static uint8_t uncompress[64*64/8];
void graph_pic(const ico_t *p, int ox, int oy) {
	int x, y;
	int size = sizeof(uncompress);
	const uint8_t *data;
	// Требуется предварительная распаковка функцией lzss_unpack():
	lzss_decode(p->data, p->size, uncompress, &size);
	y = p->sizey/8;
	if (p->sizey%8) {
		y++;
	}
	if (size != p->sizex*y) {
		DBG("lzss_decode() error: need size %db (unpack %db)\r\n", p->sizex*y, size);
		return;
	}
	data = uncompress;
	for (y=0; y<p->sizey && (oy+y) < SIZE_Y; y++) {
		for (x=0; x<p->sizex && (ox+x) < SIZE_X; x++) {
			int offset = SIZE_X * (y+oy) + (x+ox);
			if (offset > SIZE_X*SIZE_Y) {
				continue;
			}
			if (data[p->sizex*(y/8) + x] & (1<<(y&7))) {
				SET_PIXEL(ox+x,oy+y);
			}
		}
	}
}
#endif

#if !defined( __GCC__ )
// ---------------------------------------------
// EXT FONT SUPPORT
// ---------------------------------------------

//#include "./fonts/arial_30_pk.c"
//#include "./fonts/arial_30b_pk.c"
//#include "./fonts/courier_30_pk.c"
//#include "./fonts/courier_30b_pk.c"
//#include "./fonts/helv_30_pk.c"
//#include "./fonts/helv_30b_pk.c"
//#include "./fonts/sans_30_pk.c"
#include "./fonts/sans_30b_pk.c"
//#include "./fonts/times_30_pk.c"
//#include "./fonts/times_30b_pk.c"
//#include "./fonts/arial_48_pk.c"
//#include "./fonts/arial_48b_pk.c"
//#include "./fonts/courier_48_pk.c"
//#include "./fonts/courier_48b_pk.c"
//#include "./fonts/helv_48_pk.c"
//#include "./fonts/helv_48b_pk.c"
//#include "./fonts/sans_48_pk.c"
//#include "./fonts/sans_48b_pk.c"
//#include "./fonts/times_48_pk.c"
//#include "./fonts/times_48b_pk.c"

#define FONT_ARRAY font_array

//
// FONT_RECODE - определение, говорящее о том, что таблица символов 
//

#if defined( FONT_RECODE )
const uint8_t map_font32[256] = { 
//         0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
/* 0x00 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0x10 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0x20 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  11,  12,  13,  
/* 0x30 */    1,   2,   3,   4,   5,   6,   7,   8,   9,  10,   0,   0,   0,   0,   0,   0,  
/* 0x40 */    0,   0,   0,  14,  15,   0,   0,   0,   0,   0,   0,   0,   0,   0,  16,   0,  
/* 0x50 */   17,   0,  18,   0,   0,   0,  19,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0x60 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0x70 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0x80 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0x90 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0xA0 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0xB0 */   20,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0xC0 */    0,   0,  21,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0xD0 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0xE0 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
/* 0xF0 */    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
};
#endif
// Вывод строки символов высотой 32px с центрированием по указанной позиции (полутоновой шрифт)
void graph_puts32c(int ox, int oy, char *str) {
	uint8_t ch;
	int offset = 0;
	int i = 0;
	int wide = 0;
	// Рассчет выравнивания:
	while ((ch = str[i]) != 0) {
#if defined( FONT_RECODE )
		ch = map_font32[ch];
#else
		ch -= 0x20;
#endif
		wide += FONT_ARRAY[ch].sizex;
		i++;
	}
	ox -= wide/2; if (ox < 0) ox = 0;
	// Вывод строки:
	i = 0;
	while ((ch = str[i]) != 0) {
#if defined( FONT_RECODE )
		ch = map_font32[ch];
#else
		ch -= 0x20;
#endif
		if (FONT_ARRAY[ch].data == NULL) {
			//nop
		} else {
			graph_pic(&FONT_ARRAY[ch], ox+offset, oy);
		}
		offset += FONT_ARRAY[ch].sizex;
		i++;
	}
}
// ---------------------------------------------
// EXT FONT SUPPORT END
// ---------------------------------------------
#endif // __GCC__

#if !defined( __GCC__ )
// ---------------------------------------------
// LINE DRAW SUPPORT
// ---------------------------------------------
#define ABS(a) ((a)<0?-(a):(a))

static void graph_swap(int* a, int* b) {
	int t = *a; *a = *b; *b = t;
}
void graph_line(int x1, int y1, int x2, int y2) {
	int dx, dy, d, incry, incre, incrne, slopegt1 = 0;
	dx = ABS(x1-x2); dy = ABS(y1-y2);
	if(dy > dx) {
		graph_swap(&x1, &y1);
		graph_swap(&x2, &y2);
		graph_swap(&dx, &dy);
		slopegt1 = 1;
	}
	if(x1 > x2) {
		graph_swap(&x1, &x2);
		graph_swap(&y1, &y2);
	}
	if(y1 > y2) {
		incry = -1;
	} else {
		incry = 1;
	}
	d = 2*dy - dx;
	incre = 2*dy;
	incrne = 2*(dy - dx);
	while(x1 < x2) {
		if(d <= 0) {
			d+=incre;
		} else {
			d  += incrne;
			y1 += incry;
		}
		x1++;
		if(slopegt1) {
			SET_PIXEL(y1,x1);
		} else {
			SET_PIXEL(x1,y1);
		}
	}
}
// ---------------------------------------------
// LINE DRAW SUPPORT END 
// ---------------------------------------------
#endif // __GCC__

void graph_clear(void) {
	memset(frame, 0x00, sizeof(frame));
}

void graph_update(void) {
#if defined( WIN32 )
	lcd_cpframe(frame);
#else
	oled_draw(frame);
#endif
}
