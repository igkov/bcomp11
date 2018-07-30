#ifndef __OBD_H__
#define __OBD_H__

#include "can.h"

typedef struct {
	uint16_t addr;
	uint16_t pid;
	uint8_t  act;
	uint8_t  res[3];
} pid_obd_t, *ppids_obd_t;

// ������ ������� PID��:
#define PIDS_SIZE (sizeof(pids_list)/sizeof(pid_obd_t))

// �������������� OBD:
void obd_init(void);
// ���������� OBD:
void obd_deinit(void);
// ��������� ������ (��������� ��� NMPS):
void obd_act(int flag);
// ������������ ������ � ���:
void obd_getpid(uint16_t addr, uint16_t pid);
// ����������, ������ ���������� �� CAN Interrupt:
void obd_loopback(CAN_msg *p);
// ���������/����������� PID-�������:
int obd_act_set(uint16_t pid, int flag);

#endif
