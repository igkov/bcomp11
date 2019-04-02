#include <stdint.h>
#include <string.h>
#include "dbg.h"
#include "obd.h"
#include "obd_pids.h"

extern void bcomp_raw(int pid, uint8_t *data, uint8_t size);

static int state = 0;
static int val = 0;
void obd_event(void) {
	printf("obd_event(): %d\r\n", state);
	CAN_msg mess;
	uint8_t data[32];

	memset(&mess, 0, sizeof(mess));
	memset(data, 0, sizeof(data));
	
	// PIDs:
	switch (state) {
	case 0:
		data[3-1] = 105;
		bcomp_proc(INTAKE_PRESSURE, data, 10);
		break;
	case 1:
		// 30MPa * 10 = 3000
		data[3-1] = 12;
		data[4-1] = 30;
		bcomp_proc(FUEL_RAIL_PRES_ALT, data, 10);
		break;
	//case 2:
	//	data[3-1] = 55;
	//	bcomp_proc(VEHICLE_SPEED, data, 10);
	//	break;
	case 3:
		data[3-1] = 52 + (val%3);
		data[4-1] = 100;
		bcomp_proc(ECU_VOLTAGE, data, 10);
		break;
	}
	
	// RAW:
	switch (state) {
	case 0:
		mess.id = 0x0215;
		mess.data[0] = 24 + (val%10); 
		mess.data[1] = 134;
		mess.len = 8;
		break;
	case 1:
		mess.id = 0x0218;
		mess.data[2] = 0x22;
		mess.len = 8;
		break;
	case 2:
		mess.id = 0x0236;
		memset(mess.data, 0, 8);
		mess.len = 8;
		break;
	case 3:
		mess.id = 0x0308;
		mess.data[1] = 7;
		mess.data[2] = 23;
		mess.len = 8;
		break;
	case 4:
		mess.id = 0x0608;
		mess.data[0] = 60+40 + (val%10);
		mess.len = 8;
		break;
	default:
		mess.id = -1;
		break;
	}
	
	if (mess.id != -1) {
		bcomp_raw(mess.id, mess.data, mess.len);
	}
	
	if (state == 5) {
		state = 0;
	} else {
		state++;
	}
	val++;
	event_set(obd_event, 100);
}

void obd_init(void) {
	printf("obd_init()\r\n");
	event_set(obd_event, 100);
}

int obd_act_set(uint16_t pid, int flag){}
void obd_act(int act){}
void CAN_erStat(uint8_t *a, uint8_t *b){*a=5;*b=6;}
