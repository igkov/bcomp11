#ifndef __OBD_H__
#define __OBD_H__

#include "can.h"

typedef struct {
	uint16_t addr;
	uint16_t pid;
	uint8_t  act;
	uint8_t  res[3];
} pid_obd_t, *ppids_obd_t;

// Размер раблицы PIDов:
#define PIDS_SIZE (sizeof(pids_list)/sizeof(pid_obd_t))

#define OBD_DELAY_PIDS    150
#define OBD_DELAY_TIMEOUT 100
#define OBD_DELAY_START   10000

// Инцициализация OBD:
void obd_init(void);
// Отключение OBD:
void obd_deinit(void);
// Активация опроса (актуально для NMPS):
void obd_act(int flag);
// Инициировать запрос к ЭБУ:
void obd_getpid(uint16_t addr, uint16_t pid);
// Обработчик, должен вызываться из CAN Interrupt:
void obd_loopback(CAN_msg *p);
// Активация/деактивация PID-запроса:
int obd_act_set(uint16_t pid, int flag);

#endif
