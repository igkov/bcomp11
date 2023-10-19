#ifndef __DIAGRAM_H__
#define __DIAGRAM_H__

#include <stdint.h>

// Поддержка вертикальных разделителей на диаграмме:
#define DIAGRAM_DELIM_SUPPORT 0

#define DIAGRAM_SIZE_X    128
#define DIAGRAM_SIZE_Y    32
#define DIAGRAM_OFFSET_X  0
#define DIAGRAM_OFFSET_Y  32

typedef struct {
	double offset;
	float max;
	float min;
	float time;
	uint8_t points[DIAGRAM_SIZE_X];
} diagram_t, *pdiagram_t;

#if ( DIAGRAM_DELIM_SUPPORT == 1 )
int diagram_delim(pdiagram_t diag);
#endif
int diagram_add(pdiagram_t diag, float value);
int diagram_create(pdiagram_t diag, float max, float min, float time);
int diagram_draw(pdiagram_t diag);

#endif // __DIAGRAM_H__
