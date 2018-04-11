#ifndef __VIRTUINO_H__
#define __VIRTUINO_H__

#define VIRTUINO_TYPE_INT     (0)
#define VIRTUINO_TYPE_BYTE    (1)
#define VIRTUINO_TYPE_FLOAT   (2)
#define VIRTUINO_TYPE_DOUBLE  (3)
#define VIRTUINO_TYPE_STRING  (4)

#define VIRTUINO_STRING_MAX_SIZE (32)

#define VIRTUINO_VERSION_ANSWER "!C00=1.62$"

#include <stdint.h>

typedef struct {
	uint8_t id;    // Номер канала
	uint8_t type;  // Тип переменной в микропрограмме
	uint8_t vtype; // Тип переменной в протоколе
	uint8_t res;
	void *value;
} virtuino_unit_t, *pvirtuino_unit_t;

void virtuino_proc(uint8_t data);

#endif
