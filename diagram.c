#include <string.h>
#include <math.h>
#include "diagram.h"
#include "graph.h"

int diagram_create(pdiagram_t diag, float max, float min, float time) {
	memset(diag->points, 0xFF, DIAGRAM_SIZE_X);
	diag->max = max;
	diag->min = min;
	diag->time = time;
	diag->offset = 0.0f;
	return 0;
}

#if ( DIAGRAM_DELIM_SUPPORT == 1 )
int diagram_delim(pdiagram_t diag) {
	int pos;
	// Приращение смещения:
	//diag->offset += diag->time;
	// Текущая позиция:
	pos = ((int)diag->offset) % DIAGRAM_SIZE_X;
	// Предотвращение заполнения линиями:
	if (diag->points[(pos+DIAGRAM_SIZE_X-1)%DIAGRAM_SIZE_X] == 0xFE) {
		// Уже есть линия, не выставляем!
		return 0;
	}
	// Установка значения (по старой позиции):
	diag->points[pos] = 0xFE;
	// Новая ячейка:
	diag->offset = ((int)diag->offset) + 1;
	return 0;	
}
#endif

int diagram_add(pdiagram_t diag, float value) {
	int pos;
	// Приращение смещения:
	diag->offset += diag->time;
	// Текущая позиция:
	pos = ((int)diag->offset) % DIAGRAM_SIZE_X;
	// Рассчет точки:
	if (isnan(value)) {
		diag->points[pos] = 0xFC; // unk
	} else if (value < diag->min) {
		diag->points[pos] = 0xFF; // min
	} else if (value > diag->max) {
		diag->points[pos] = 0xFD; // max
	} else {
		float x;
		x = value - diag->min;
		x /= (diag->max - diag->min);
		diag->points[pos] = (uint8_t)((1.0f - x) * (float)DIAGRAM_SIZE_Y);
	}
	return 0;
}

int diagram_draw(pdiagram_t diag) {
	int x;
	for	(x=0; x<DIAGRAM_SIZE_X; x++) {
		int pos = ((int)diag->offset + 1 + x) % DIAGRAM_SIZE_X;
#if ( DIAGRAM_DELIM_SUPPORT == 1 )
		if (diag->points[pos] == 0xFE) {
			for (y=0; y<DIAGRAM_SIZE_Y; y++) {
				graph_putpix(
					DIAGRAM_OFFSET_X+x, 
					DIAGRAM_OFFSET_Y+y, 
					0xC0);
			}
		} else
#endif
		if (diag->points[pos] == 0xFF) {
			// NOP (min)
			graph_putpix(
				DIAGRAM_OFFSET_X+x, 
				DIAGRAM_OFFSET_Y+DIAGRAM_SIZE_Y-1, 
				0x00);
		} else
		if (diag->points[pos] == 0xFD) {
			// NOP (max)
			graph_putpix(
				DIAGRAM_OFFSET_X+x, 
				DIAGRAM_OFFSET_Y+0, 
				0x00);
		} else
		if (diag->points[pos] == 0xFC) {
			// NOP
		} else {
			graph_putpix(
				DIAGRAM_OFFSET_X+x, 
				DIAGRAM_OFFSET_Y+diag->points[pos], 
				0x00);
		}
	}
	return 0;
}
