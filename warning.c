#include <stdint.h>
#include <string.h>
#include <math.h>
#include "event.h"
#include "buttons.h"
#include "dbg.h"
#include "graph.h"
#include "bcomp.h"
#include "warning.h"
#include "beep.h"
#include "errors.h"

static ecu_error_t errors_list[WARN_MAX_NUM];

/*
	Проверка выхода параметров за допустимые значения.
	Выставление ошибок.
 */
void bcomp_warning(void) {
	// CHECK CONNECT
	if (bcomp.connect == 0) {
		if (errors_list[WARNING_ID_CONNECT].flags & WARN_FLAG_ACT) {
			errors_list[WARNING_ID_CONNECT].time += 5;
			if (errors_list[WARNING_ID_CONNECT].time > bcomp.setup.w_delay*60) {
				errors_list[WARNING_ID_CONNECT].flags &= ~WARN_FLAG_HIDE;
				errors_list[WARNING_ID_CONNECT].time = 0;
			}
		} else {
			errors_list[WARNING_ID_CONNECT].time++;
			// Больше 3х раз нет флага соединения:
			if (errors_list[WARNING_ID_CONNECT].time > 3) {
				errors_list[WARNING_ID_CONNECT].flags |= WARN_FLAG_ACT;
				errors_list[WARNING_ID_CONNECT].time = 0;
			}
		}
		// Возвращаемся, другие ошибки пока мониторить бессмысленно.
		goto bcomp_warning_end;
	} else {
		errors_list[WARNING_ID_CONNECT].flags = 0;
		errors_list[WARNING_ID_CONNECT].time = 0;
	}
	// CHECK ENGINE
	if (bcomp.mil) {
		if (errors_list[WARNING_ID_ENGINE].flags & WARN_FLAG_ACT) {
			errors_list[WARNING_ID_ENGINE].time += 5;
			if (errors_list[WARNING_ID_ENGINE].time > bcomp.setup.w_delay*60) {
				errors_list[WARNING_ID_ENGINE].flags &= ~WARN_FLAG_HIDE;
				errors_list[WARNING_ID_ENGINE].time = 0;
			}
		} else {
			errors_list[WARNING_ID_ENGINE].flags |= WARN_FLAG_ACT;
			errors_list[WARNING_ID_ENGINE].time = 0;
		}
	} else {
		errors_list[WARNING_ID_ENGINE].flags = 0;
		errors_list[WARNING_ID_ENGINE].time = 0;
	}
	// ENGINE TEMPERATURE WARNING
	if (bcomp.t_engine > bcomp.setup.t_eng) {
		if (errors_list[WARNING_ID_T_ENGINE].flags & WARN_FLAG_ACT) {
			errors_list[WARNING_ID_T_ENGINE].time += 5;
			if (errors_list[WARNING_ID_T_ENGINE].time > bcomp.setup.w_delay*60) {
				errors_list[WARNING_ID_T_ENGINE].flags &= ~WARN_FLAG_HIDE;
				errors_list[WARNING_ID_T_ENGINE].time = 0;
			}
		} else {
			errors_list[WARNING_ID_T_ENGINE].flags |= WARN_FLAG_ACT;
			errors_list[WARNING_ID_T_ENGINE].time = 0;
		}
	} else 
	if (bcomp.t_engine < bcomp.setup.t_eng-3) {
		errors_list[WARNING_ID_T_ENGINE].flags = 0;
		errors_list[WARNING_ID_T_ENGINE].time = 0;
	}
	// AT EMPERATURE WARNING
	if (bcomp.at_present) {
		if (bcomp.t_akpp == 0xFFFF) {
			// NOP
		} else
		if (bcomp.t_akpp > bcomp.setup.t_at) {
			if (errors_list[WARNING_ID_T_AT].flags & WARN_FLAG_ACT) {
				errors_list[WARNING_ID_T_AT].time += 5;
				if (errors_list[WARNING_ID_T_AT].time > bcomp.setup.w_delay*60) {
					errors_list[WARNING_ID_T_AT].flags &= ~WARN_FLAG_HIDE;
					errors_list[WARNING_ID_T_AT].time = 0;
				}
			} else {
				errors_list[WARNING_ID_T_AT].flags |= WARN_FLAG_ACT;
				errors_list[WARNING_ID_T_AT].time = 0;
			}
		}
	} else 
	if (bcomp.t_akpp < bcomp.setup.t_at-3) {
		errors_list[WARNING_ID_T_AT].flags = 0;
		errors_list[WARNING_ID_T_AT].time = 0;
	}
	// BATTERY WARNING
	if (isnan(bcomp.v_ecu)) {
		// NOP
	} else
	if (bcomp.v_ecu > bcomp.setup.v_max ||
		bcomp.v_ecu < bcomp.setup.v_min) {
		if (errors_list[WARNING_ID_BATTERY].flags & WARN_FLAG_ACT) {
			errors_list[WARNING_ID_BATTERY].time += 5;
			if (errors_list[WARNING_ID_BATTERY].time > bcomp.setup.w_delay*60) {
				errors_list[WARNING_ID_BATTERY].flags &= ~WARN_FLAG_HIDE;
				errors_list[WARNING_ID_BATTERY].time = 0;
			}
		} else {
			errors_list[WARNING_ID_BATTERY].time++;
			// Больше 3х раз подряд просадка напряжения!
			if (errors_list[WARNING_ID_BATTERY].time > 3) {
				errors_list[WARNING_ID_BATTERY].flags |= WARN_FLAG_ACT;
				errors_list[WARNING_ID_BATTERY].time = 0;
			}
		}
	} else 
	if (bcomp.v_ecu < bcomp.setup.v_max-0.4f &&
		bcomp.v_ecu > bcomp.setup.v_min+0.4f) {
		errors_list[WARNING_ID_BATTERY].flags = 0;
		errors_list[WARNING_ID_BATTERY].time  = 0;
	}
	// EXT TEMP WARNING
	if (bcomp.setup.f_ext_w && bcomp.setup.f_ext) {
		if (bcomp.t_ext == 0xFFFF) {
			// NOP
		} else
		if (bcomp.t_ext < bcomp.setup.t_ext) {
			if (errors_list[WARNING_ID_T_EXT].flags & WARN_FLAG_ACT) {
				// Самостоятельно гасет через 10 секунд:
				errors_list[WARNING_ID_T_EXT].time += 5;
				if (errors_list[WARNING_ID_T_EXT].time >= 10) {
					errors_list[WARNING_ID_T_EXT].flags |= WARN_FLAG_HIDE;
				}
			} else {
				errors_list[WARNING_ID_T_EXT].flags |= WARN_FLAG_ACT;
			}
		}
	} else 
	if (bcomp.t_ext > bcomp.setup.t_ext+3) {
		errors_list[WARNING_ID_T_EXT].flags = 0;
		errors_list[WARNING_ID_T_EXT].time  = 0;
	}
	// GPS WARNING
	if (bcomp.setup.f_gps) {
		if (bcomp.nmea_cnt < 100 ||
			bcomp.g_correct == 0) {
			if (errors_list[WARNING_ID_GPS].flags & WARN_FLAG_ACT) {
				errors_list[WARNING_ID_GPS].time += 5;
				if (errors_list[WARNING_ID_GPS].time > bcomp.setup.w_delay*60) {
					errors_list[WARNING_ID_GPS].flags &= ~WARN_FLAG_HIDE;
					errors_list[WARNING_ID_GPS].time = 0;
				}
			} else {
				errors_list[WARNING_ID_GPS].time++;
				// Ошибка по GPS только через 5 минут:
				if (errors_list[WARNING_ID_GPS].time > 60) {
					errors_list[WARNING_ID_GPS].flags |= WARN_FLAG_ACT;
					errors_list[WARNING_ID_GPS].time = 0;
				}
			}
		}
	}
	// SERVICE
	if ((int)bcomp.moto_dist_service > bconfig.service_distation*1000 ||
		bcomp.moto_time_service > bconfig.service_motohour*3600) {
		if (errors_list[WARNING_ID_SERVICE].flags & WARN_FLAG_ACT) {
			// nop
		} else {
			errors_list[WARNING_ID_SERVICE].flags |= WARN_FLAG_ACT;
			errors_list[WARNING_ID_SERVICE].time = 0;
		}
	}
	// FUEL
	if (bcomp.setup.f_fuel) {
		if (bcomp.fuel_level < bcomp.setup.l_fuel) {
			if (errors_list[WARNING_ID_FUEL].flags & WARN_FLAG_ACT) {
				errors_list[WARNING_ID_FUEL].time += 5;
				if (errors_list[WARNING_ID_FUEL].time > bcomp.setup.w_delay*60) {
					errors_list[WARNING_ID_FUEL].flags &= ~WARN_FLAG_HIDE;
					errors_list[WARNING_ID_FUEL].time = 0;
				}
			} else {
				errors_list[WARNING_ID_FUEL].time++;
				// Ошибка по FUEL появляется с задержкой в 1 минуту,
				// фильтрует мгновенные уменьшения уровня ниже минимума 
				// во время движения:
				if (errors_list[WARNING_ID_FUEL].time > 12) {
					errors_list[WARNING_ID_FUEL].flags |= WARN_FLAG_ACT;
					errors_list[WARNING_ID_FUEL].time = 0;
				}
			}
		}
	}
bcomp_warning_end:
	// Раз в 5 секунд:
	event_set(bcomp_warning, 5000);
}

#if ( GRAPH_SUPPORT == 1 )
// -----------------------------------------------------------------------------
// show_XXX
// Функции отображения информации.
// -----------------------------------------------------------------------------
int warning_show(int *act) {
	uint8_t id = 0;
	char str[10];
	for (id = 0; id < WARN_MAX_NUM; id++) {
		if (errors_list[id].flags & WARN_FLAG_ACT) {
			if ((errors_list[id].flags & WARN_FLAG_HIDE) == 0) {
				break;
			}
		}
	}
	if (*act & BUTT_SW1) {
		// Скрытие ошибки, флаг ошибки не сбрасывается,
		// после прохода заданного времени, ошибка возникает вновь!
		errors_list[id].flags |= WARN_FLAG_HIDE;
	} else 
	if (*act & BUTT_SW1_LONG) {
		// Погасим саму ошибку.
		errors_list[id].flags = 0;
		if (id == 0) {
			// TODO:
			// Для CHECK ENGINE здесь можно поставить логику сброса ошибки.
		}
		return 1;
	}
	switch (id) {
	case WARNING_ID_CONNECT:
		graph_pic(&ico48_connect,64-24,0);
		graph_puts16(64,48,1,"NO CAN!");
		break;
	case WARNING_ID_ENGINE:
		graph_pic(&ico48_engine,64-24,0);
		error_decrypt(bcomp.e_code, str);
		graph_puts16(64,48,1,str);
		break;
	case WARNING_ID_T_ENGINE:
		graph_pic(&ico48_trans,64-24,0);
		_sprintf(str, "%d°C", bcomp.t_engine);
		graph_puts16(64,48,1,str);
		break;
	case WARNING_ID_T_AT:
		graph_pic(&ico48_temp,64-24,0);
		_sprintf(str, "%d°C", bcomp.t_akpp);
		graph_puts16(64,48,1,str);
		break;
	case WARNING_ID_BATTERY:
		graph_pic(&ico48_battery,64-24,0);
		_sprintf(str, "%d.%dV", (int)bcomp.v_ecu, (int)(bcomp.v_ecu*10)%10);
		graph_puts16(64,48,1,str);
		break;
	case WARNING_ID_T_EXT:
		graph_pic(&ico48_silkroad,64-24,0);
		_sprintf(str, "%d°C", (int)bcomp.t_ext);
		graph_puts16(64,48,1,str);
		break;
	case WARNING_ID_GPS:
		graph_pic(&ico48_gps,64-24,0);
		if (bcomp.nmea_cnt < 100) {
			graph_puts16(64,48,1,"NO DATA");
		} else {
			graph_puts16(64,48,1,"NO SAT");
		}
		break;
	case WARNING_ID_SERVICE:
		graph_pic(&ico48_service,64-24,0);
		graph_puts16(64,48,1,"GO SERVICE");
		break;
	default:
		graph_puts16(64,48,1,"UNKNOWN");
		break;
	}
	return 0;
}
#endif

void warning_check(void) {
	int i;
	for (i = 0; i < WARN_MAX_NUM; i++) {
		if (errors_list[i].flags & WARN_FLAG_ACT) {
			if ((errors_list[i].flags & WARN_FLAG_HIDE) == 0) {
#if !defined( WIN32 )
				if (beep_is_play() == 0 &&
					(bcomp.page & GUI_FLAG_WARNING) == 0) {
					beep_play(melody_warning);
				}
#endif
				// Выставляем ошибку:
				bcomp.page |= GUI_FLAG_WARNING;
				return;
			}
		}
	}
	// Нет предупреждений!
	bcomp.page &= ~GUI_FLAG_WARNING;
}

void warning_init(void) {
	memset(errors_list, 0x00, sizeof(errors_list));
}
