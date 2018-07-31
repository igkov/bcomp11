/* 
	Упрощенный менеджер. Запросы через фиксированные интервалы времени.
 */

#include <stdint.h>
#include <string.h>
#include "event.h"
#include "obd.h"
#include "obd_pids.h"
#include "dbg.h"
#include "bcomp.h"

// Переменные обработчика
static uint16_t can_pid;
static uint16_t can_id;
static uint8_t can_len;
static uint8_t can_offset;
static uint8_t can_cnt;
static uint8_t can_buffer[128];
static uint8_t can_done;
// Переменная для "карусели":
static int count;

/* Список PID-ов для получения. */
pid_obd_t pids_list[] = {
	// CAN ADDR          | PID              | ACT |  RES1  RES2  RES3     
#if ( PAJERO_SPECIFIC == 0 )
	{ PID_REQUEST_ENGINE,  ENGINE_RPM,           1, {0xFF, 0xFF, 0xFF} }, // 1. Обороты (получаем сырыми данными).
	{ PID_REQUEST_ENGINE,  VEHICLE_SPEED,        1, {0xFF, 0xFF, 0xFF} }, // 2. Спидометр (получаем сырыми данными).
	{ PID_REQUEST_ENGINE,  ENGINE_COOLANT_TEMP,  1, {0xFF, 0xFF, 0xFF} }, // 3. Темп. двигателя (получаем сырыми данными).
#endif
	{ PID_REQUEST_ENGINE,  ECU_VOLTAGE,          1, {0xFF, 0xFF, 0xFF} }, // 4. Напряжение бортовой сети.
#if ( PAJERO_SPECIFIC == 1 )
	{ PID_REQUEST_AT,      PAJERO_AT_INFO,       1, {0xFF, 0xFF, 0xFF} }, // 5. Темп. коробки (MITSUBISHI)
#elif ( NISSAN_SPECIFIC == 1 )
	{ PID_REQUEST_AT,      NISSAN_AT_INFO,       1, {0xFF, 0xFF, 0xFF} }, // 5. Темп. коробки (NISSAN)
#endif
	{ PID_REQUEST_ENGINE,  GET_VIN,              1, {0xFF, 0xFF, 0xFF} }, // 6. Получение VIN-автомобиля.
	{ PID_REQUEST_ENGINE,  INTAKE_PRESSURE,      1, {0xFF, 0xFF, 0xFF} }, // 7. Давление наддува.
	//{ PID_REQUEST_ENGINE,  ENGINE_RUNTIME,       0, {0xFF, 0xFF, 0xFF} }, // 8. Время работы двигателя.
	{ PID_REQUEST_ENGINE,  FUEL_RAIL_PRES_ALT,   1, {0xFF, 0xFF, 0xFF} }, // 9. Давление топлива в рейке.
	{ PID_REQUEST_ENGINE,  ECU_VOLTAGE,          1, {0xFF, 0xFF, 0xFF} }, // 10. Напряжение бортовой сети.
	//{ PID_REQUEST_ENGINE,  MAF_SENSOR,           0, {0xFF, 0xFF, 0xFF} }, // 11. Данные с MAF-сенсора.
	//{ PID_REQUEST_ENGINE,  BAROMETRIC_PRESSURE,  0, {0xFF, 0xFF, 0xFF} }, // 12. Наружное давление.
#if 0
	{ PID_REQUEST_ENGINE,  STATUS_DTC,           1, {0xFF, 0xFF, 0xFF} }, // 13. Информация об ощибках в ЭБУ двигателя.
	{ PID_REQUEST_ENGINE,  FREEZE_DTC,           0, {0xFF, 0xFF, 0xFF} }, // 14. Получение кода ошибки из памяти (отключено по-умолчанию).
#endif
#if ( PAJERO_SPECIFIC == 1 )
	{ PID_REQUEST_ENGINE,  PAJERO_ODO_INFO,      0, {0xFF, 0xFF, 0xFF} }, // 15. Получение данных одометра.
#endif
};

// Обработчик пакетов от шины CAN (по протоколу J1979):
void obd_loopback(CAN_msg *p) {
	if (can_id != p->id) {
		// Сырые пакеты данных:
		bcomp_raw(p->id, p->data, p->len);
		return;
	}
	if (p->len != 8) {
		DBG("ERROR: incorrect LEN in CAN frame, LEN = %d!\r\n", p->len);
		return;
	}
	if (can_len > can_offset) {
		if (can_cnt == p->data[0]) {
			int len = (can_len-can_offset)>7?7:(can_len-can_offset);

			if (can_offset+len <= sizeof(can_buffer)) {
				memcpy(&can_buffer[can_offset], &p->data[1], len);
			}
			
			can_offset += len;
			can_cnt++;
			
			if (can_offset == can_len) {
				goto obd_loopback_recv;
			}
		}
		else {
			DBG("Bad data sequence (%02x)!\r\n", p->data[0]);
		}
	}
	else if (p->data[0] == 0x10) {
		if (p->data[2] != (0x40+(can_pid>>8))) {
			DBG("ERROR: incorrect Mode in answer (wait Mode=%02x, received Mode=%02x)!\r\n", can_pid, p->data[2]);
			return;
		}
		if (p->data[3] != (can_pid&0xFF)) {
			DBG("ERROR: incorrect PID in answer (wait PID=%02x, received PID=%02x)!\r\n", can_pid, p->data[3]);
			return;
		}
		memset(can_buffer, 0, sizeof(can_buffer));
		memcpy(can_buffer, &p->data[2], 6);
		
		can_len = p->data[1];
		can_offset = 6;
		can_cnt = 0x21;

		// get more:
		memset(CAN_TxMsg.data, 0x00, 8);
		CAN_TxMsg.data[0] = 0x30;
		CAN_TxMsg.len = 8;
		CAN_TxMsg.format = STANDARD_FORMAT;
		CAN_TxMsg.type = DATA_FRAME;
		//CAN_writeMsg(1, &CAN_TxMsg);
		CAN_wrMsg(&CAN_TxMsg);
	}
	else if (p->data[0] >= 2 &&
			 p->data[0] <= 7 ) {
		if (p->data[1] != (0x40+(can_pid>>8))) {
			DBG("ERROR: incorrect Mode in answer (wait %02x, receive %02x)!\r\n", can_pid, p->data[1]);
			return;
		}
		if (p->data[2] != (can_pid&0xFF)) {
			DBG("ERROR: incorrect PID in answer (wait %02x, receive %02x)!\r\n", (can_pid&0xFF), p->data[2]);
			return;
		}
		// Копируем:
		can_len = p->data[0];
		can_offset = p->data[0];
		
		memset(can_buffer, 0, sizeof(can_buffer));
		memcpy(can_buffer, &p->data[1], can_len);
obd_loopback_recv:
		// Завершились, флаг готовности данных:
		can_done = 1;
	}
	else {
		DBG("ERROR: incorrect lenght in answer!\r\n");
		return;
	}
}

void obd_getpid(uint16_t addr, uint16_t pid) {
	//DBG("get_pid(%04x, %04x);\r\n", addr, pid);
#if ( 0 )
	// Настройка приема:
	CAN_wrFilter(addr+0x08, STANDARD_FORMAT);
#endif
	// Инициализация обработчика:
	can_len = 0;
	can_id = addr+0x08;
	can_pid = pid;
	can_offset = 0;
	can_cnt = 0;
	can_done = 0;
	// Собираем запрос:
	CAN_TxMsg.id = addr;
	memset(CAN_TxMsg.data, 0x00, 8);
	CAN_TxMsg.data[0] = 0x02;
	CAN_TxMsg.data[1] = pid>>8;
	CAN_TxMsg.data[2] = pid&0xFF;
	CAN_TxMsg.len = 8;
	CAN_TxMsg.format = STANDARD_FORMAT;
	CAN_TxMsg.type = DATA_FRAME;
	// Отправка запроса:
	//CAN_writeMsg(1, &CAN_TxMsg);
	CAN_wrMsg(&CAN_TxMsg);
}

static void obd_manage(void) {
	if (can_done == 1) {
		// Запрос обработан!
		bcomp_proc(can_pid, can_buffer, can_len);
		//Свободны, обработка след. запроса:
		can_pid = can_id = can_done = 0;
		//Задержка перед следующей обработкой:
		event_set(obd_manage, 200);
	} else 
	if (can_pid == 0) {
		// В случае установленного сервисного флага, запросы в ЭБУ не отправляются.
		// Пассивное прослушивание шины при этом остается активным.
		if (bcomp.service == 0) {
			// Запрос к ЭБУ.
			obd_getpid(pids_list[count%PIDS_SIZE].addr, pids_list[count%PIDS_SIZE].pid);
		}
		// Позиция для следующего запроса:
		do {
			count++;
		} while (pids_list[count%PIDS_SIZE].act == 0); 
		// Запрос отправлен, ожидаем до 100мс:
		event_set(obd_manage, 100);
		return;
	} else {
		// Таймаут!
		can_pid = 0;
		event_set(obd_manage, 10);
	}
}

void obd_init(void) {
	// Start Rs signal:
	CAN_rs_set();
	// Обнуляем переменные:
	can_pid = 0;
	can_len = 0;
	can_offset = 0;
	can_cnt = 0;
	can_done = 0;
#if ( PAJERO_SPECIFIC == 1 )
	count = -1;
#else
	count = 0;
#endif
	memset(can_buffer, 0, sizeof(can_buffer));
	// Инициализируем CAN-интерфейс:
	CAN_setup(bconfig.can_speed);                  /* setup CAN Controller, 500kbit/s */
	CAN_start();                                   /* start CAN Controller    */
	CAN_waitReady();                               /* wait til tx mbx is empty */
	CAN_noFilter(STANDARD_FORMAT);
#if ( PAJERO_SPECIFIC == 0 )
	// 5000ms обработка, задержка 5 секунд перед началом опроса ECU:
	event_set(obd_manage, 5000);
#endif
}

void obd_act(int flag) {
	if (flag) {
		if (count == -1) {
			count = 0;
			// задержка 5 секунд перед началом опроса ECU:
			event_set(obd_manage, 5000);
		}
	} else {
		count = -1;
		event_unset(obd_manage);
	}
}

void obd_deinit(void) {
	// Deinit CAN:
	CAN_rs_unset();
	CAN_init_low();
	CAN_stop();
}

int obd_act_set(uint16_t pid, int flag) {
	int i;
	for (i=0; i<PIDS_SIZE; i++) {
		if (pids_list[i].pid == pid) {
			pids_list[i].act = flag;
		}
	}
	return 1;
}
