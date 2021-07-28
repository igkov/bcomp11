/*
	BCOMP11 v2 firmware
	
	Асинхронные события:
	1) button
	2) calc
	3) beep
	4) save
	5) obd
	6) analog
	7) elog
	8) warn
    9) rpm_flag_check
	
	Экраны:
	1) Основной экран с передачей и пробегом. Так же возможен вывод температуры.
	2) Температура двигателя.
	3) Температура трансмиссии (АКПП).
	4) Напряжение бортовой сети.
	5) Расходы топлива.
	6) Путь А.
	7) Путь Б.
	8) Сервисные интервалы.
	9) Положение колес.
	10) GPS-данные.
	11) Давление в топливной рейке.
	12) Давление во впускном коллекторе.
    13) Счетчик ошибок на шине.
    14) Общий экран с основными параметрами.
    15) Рассветы/закаты.
	
	Вторая итерация прошивки мини-бортового компьютера.
	Обточка функционала для реализации полноценного устройства.
	
	igorkov / 2017-2018 / igorkov.org/bcomp11v2
 */
#if defined( WIN32 )
#include <stdint.h>
#include <stdio.h>
#include "winlcd.h"
#include "windows.h"
#include "windowsx.h"
#include "string.h"
#include "Shellapi.h"
#else
#include <LPC11xx.h>
#endif
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "bcomp.h"
#include "leds.h"
#include "event.h"
#include "adc.h"
#include "analog.h"
#include "beep.h"
#include "buttons.h"
#include "dbg.h"
#include "i2c.h"
#include "bmp085.h"
#include "eeprom.h"
#include "uart0.h"

#include "icons.h"
#include "graph.h"
#include "menu.h"
#include "wheels.h"
#include "drive.h"
#include "diagram.h"
#include "lowpass.h"

#include "obd.h"
#include "obd_pids.h"
#include "oled128.h"
#include "config.h"
#include "errors.h"
#include "elog.h"
#include "virtuino.h"
#include "warning.h"
#include "nmea.h"

#include "sun.h"
#include "moon.h"

#if defined( WIN32 )
#define __WFI() Sleep(1)
#define delay_mks(a) Sleep(a/1000)
#define delay_ms(a) Sleep(a)
#endif

#define BCOMP_PAGE()       (bcomp.page & 0x0FFF)
#define BCOMP_PAGE_PREV()  (bcomp.page = ((bcomp.page & 0xF000) | ((bcomp.page - 1) & 0x0FFF)))
#define BCOMP_PAGE_NEXT()  (bcomp.page = ((bcomp.page & 0xF000) | ((bcomp.page + 1) & 0x0FFF)))

bcomp_t bcomp;
volatile uint8_t i2c_busy = 0;

// -----------------------------------------------------------------------------
// exeption_proc
// Попадание в эту функцию говорит о серьезной ошибке.
// -----------------------------------------------------------------------------
void exeption_proc(void) {
	DBG("exeption_proc(): while (1);\r\n");
	while (1) {
		delay_ms(300);
		led_red(1);
		delay_ms(300);
		led_red(0);
	}
}

void bcomp_press(void);
void bcomp_press_read(void);
void bcomp_temp(void);
void bcomp_temp_read(void);

void bcomp_temp(void) {
    if (i2c_busy == 0) {
        bmp085_start_temp();
        event_set(bcomp_temp_read, 20);
    } else {
        event_set(bcomp_temp, 10);
    }
}

void bcomp_temp_read(void) {
    if (i2c_busy == 0) {
        if (bmp085_get_temp(&bcomp.temp)) {
            bcomp.temp = 0xFFFF;
        } else {
            diagram_add(&bcomp.dia_temp, (float)bcomp.temp/10);
        }
        event_set(bcomp_press, 10);
    } else {
        event_set(bcomp_temp_read, 10);
    }
}

void bcomp_press(void) {
    if (i2c_busy == 0) {
        bmp085_start_baro(3);
        event_set(bcomp_press_read, 100);
    } else {
        event_set(bcomp_press, 10);
    }
}

void bcomp_press_read(void) {
    if (i2c_busy == 0) {
        if (bmp085_get_baro(&bcomp.press)) {
            // error
            bcomp.press = 0xFFFF;
            bcomp.mmhg = NAN;
            bcomp.height = NAN;
        } else {
            if (bcomp.filt_p.xp == 0) {
                bcomp.filt_p.max_d = 100.0f;
                bcomp.filt_p.xp = bcomp.press;
                bcomp.filt_p.a = 0.1f;
            }
            bcomp.press = lowpass((float)bcomp.press, &bcomp.filt_p);
            pressure_convert(bcomp.press, &bcomp.mmhg, &bcomp.height);
            
            diagram_add(&bcomp.dia_height, (float)bcomp.height);
            diagram_add(&bcomp.dia_press, (float)bcomp.press);
        }
        event_set(bcomp_temp, 300);
    } else {
        event_set(bcomp_press_read, 10);
    }
}

// -----------------------------------------------------------------------------
// bcomp_XXX
// Асинхронные функции обработки различных событий.
// -----------------------------------------------------------------------------
#if !defined( WIN32 )
void ProtectDelay(void) {
	int n;
	for (n = 0; n < 100000; n++) { __NOP(); } 
}
#endif

#if defined( WIN32 )
// Win-версия запускает основную логику в дополнительном потоке:
DWORD WINAPI ProcMain(LPVOID par)
#else
int main(void)
#endif
{
	int ret;
	int ms;
	char str[20];

	// Инициализация периферийных модулей и библиотек:
	leds_init();
	led_red(1);
    led_green(0);
	//uart0_init(bconfig.uart_speed);
	event_init();
	button_init();
	beep_init();
	i2c_init();
    bmp085_init();
    led_green(1);
	//adc_init();
	DBG("init ok!\r\n");

	bcomp.filt_p.xp = 0;

	// -----------------------------------------------------------------------------
	// Инициализация переменных:
	// -----------------------------------------------------------------------------
	bcomp.time = 0;
	bcomp.page = 1;
	bcomp.press = 0xFFFF;
	bcomp.height = NAN;
    bcomp.temp = 0xFFFF;
    bcomp.mmhg = NAN;

	// -----------------------------------------------------------------------------
	// Загрузка сохраненных значений и настроек:
	// -----------------------------------------------------------------------------

	diagram_create(&bcomp.dia_height, -50, 350, 1);
	diagram_create(&bcomp.dia_press, 95000, 105000, 1); 
	diagram_create(&bcomp.dia_temp, 0, 50, 1); 
	
	// -----------------------------------------------------------------------------
	// Инициализируем асинхронные события:
	// -----------------------------------------------------------------------------

	// Сохраняем данные в EEPROM, только если задана её конфигурация:
	event_set(bcomp_temp, 1000); //delay_ms(10);

	// -----------------------------------------------------------------------------
	// Инициализируем экран:
	// -----------------------------------------------------------------------------

#if ( GRAPH_SUPPORT == 1 )
	oled_init(bcomp.setup.contrast, 0);
	graph_clear();
#endif

	// -----------------------------------------------------------------------------
	// Запускаем OBD-протокол:
	// -----------------------------------------------------------------------------
	//obd_init();

	// -----------------------------------------------------------------------------
	// Стартовый экран (заставка/мелодия):
	// -----------------------------------------------------------------------------

#if ( GRAPH_SUPPORT == 1 )
	if (bconfig.start_delay) {
#if ( PAJERO_SPECIFIC == 1 )
		// Новая версия заставки, только иконка:
		graph_pic(&ico64_mitsu,64-32,0);
#elif ( NISSAN_SPECIFIC == 1 )
		// Новая версия заставки, только иконка:
		graph_pic(&ico64_nissan,64-36,8);
#else
		// Просто картинка заставки:
		graph_pic(&ico48_mcu,64-24,8);
#endif
		graph_update();
	}
#endif
	if (bconfig.start_sound) {
#if !defined( WIN32 )
		beep_play(melody_start);
#endif
	}
	if (bconfig.start_delay) {
		// Ожидание N секунд, по нажатию - прерываем.
		ms = get_ms_timer();
		while ((get_ms_timer() - ms) < bconfig.start_delay*1000) {
			__WFI();
			ret = button_read();
			if (ret) {
				break;
			}
		}
	} else {
		// Задержка перед основной логикой:
		delay_ms(1000);
	}

	// Инициализация закончена, выключаем красный светодиод:
	led_red(0);
		
	// -----------------------------------------------------------------------------
	// Основной цикл работы интерфейса:
	// -----------------------------------------------------------------------------
	ret = 0;
	while (1) {
		int buttons = 0;
#if ( GRAPH_SUPPORT == 1 )
		graph_clear();
#endif
		// Состояние кнопок:
		ms = get_ms_timer();
		while ((get_ms_timer() - ms) < 400) {
			__WFI();
			buttons = button_read();
			if (buttons) {
				break;
			}
		}
		// Вывод страницы:
		DBG("bcomp.page = %d\r\n", bcomp.page);
		// Обработка кнопок:
		if (buttons & BUTT_SW1) {
			DBG("buttons(): BUTT_SW1\r\n");
			bcomp.page &= ~GUI_FLAG_GRAPH;
#if !defined( WIN32 )
			if (bcomp.setup.sound) {
				// Нажатие на кнопку:
				beep_play(melody_wrep);
			}
#endif
			if (bcomp.page & GUI_FLAG_MENU) {
				// nop
			} else
			{
				bcomp.page++;
			}
		}
		if (buttons & BUTT_SW1_LONG) {
			DBG("buttons(): BUTT_SW1_LONG\r\n");
#if !defined( WIN32 )
			if (bcomp.setup.sound) {
				// Нажатие на кнопку:
				beep_play(melody_wrep2);
			}
#endif
			if (bcomp.page & GUI_FLAG_MENU) {
				// nop
			} else 
			if (bcomp.page == 1) {
				buttons = 0;
				bcomp.page ^= GUI_FLAG_GRAPH;
			} else 
			if (bcomp.page == 2) {
				bcomp.page ^= GUI_FLAG_GRAPH;
			} else 
			if (bcomp.page == 3) {
				bcomp.page ^= GUI_FLAG_GRAPH;
			} else
			if (bcomp.page == 4) {
				bcomp.page |= GUI_FLAG_MENU;
			} else
            {
                // nop
            }
		}
//end_sw1_long_proc:
		if (buttons & BUTT_SW2) {
			DBG("buttons(): BUTT_SW2\r\n");
			bcomp.page &= ~GUI_FLAG_GRAPH;
#if !defined( WIN32 )
			if (bcomp.setup.sound) {
				// Нажатие на кнопку:
				beep_play(melody_wrep);
			}
#endif
			if (bcomp.page & GUI_FLAG_MENU) {
				// nop
			}
                bcomp.page--;
		}
repeate:
#if ( GRAPH_SUPPORT == 1 )
		if (bcomp.page & GUI_FLAG_MENU) {
			int contrast;
			contrast = bcomp.setup.contrast;
			ret = menu_work(&buttons);
			// Проверяем, не изменился ли контраст:
			if (contrast != bcomp.setup.contrast) {
				// Изменился, перенастраиваем:
				oled_contrast(bcomp.setup.contrast);
			}
			switch (ret) {
			case 0x00:
				bcomp.page &= ~GUI_FLAG_MENU;
				goto repeate;
			case 0xF0:
				if (buttons & (BUTT_SW1|BUTT_SW1_LONG)) {
					menu_back();
				}
				graph_puts16(64,  0, 1, INFO_DEVICE);
				graph_puts16(64, 16, 1, INFO_VERSION);
				graph_puts16(64, 32, 1, INFO_AUTHOR);
				graph_puts16(64, 48, 1, INFO_YEAR);
				break;
			default:
				break;
			}
		} else {
			switch (BCOMP_PAGE()) {
			// SCREENS:
			//  1 - ODOMETER
			//  2 - ENGINE
			//  3 - TRANSMISSION (if present)
			//  4 - BATTERY
			//  5 - FUEL ECONOMY
			//  6 - TRIP A
			//  7 - TRIP B
			//  8 - SERVICE
			//  9 - WHEELS (if present)
			// 10 - GPS (if present)
			// 11 - RAIL PRESSURE
			// 12 - INTAKE PRESSURE
            // 13 - TEST CAN ERROR COUNTERS
			case 1:
				// -----------------------------------------------------------------
				// Pressure
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "PRESS");
				if (bcomp.press == 0xFFFF) {
					_sprintf(str, "--Pa");
				} else {
					_sprintf(str, "%dPa", bcomp.press);
				}
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_press);
				} else {
					graph_puts32c(64, 24, str);
				}
                break;
			case 2:
				// -----------------------------------------------------------------
				// Height
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "HEIGHT");
				if (isnan(bcomp.height)) {
					_sprintf(str, "--m");
				} else {
					_sprintf(str, "%dm", (int)bcomp.height);
				}
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_height);
				} else {
					graph_puts32c(64, 24, str);
				}
				break;
			case 3:
				// -----------------------------------------------------------------
				// Temp
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "TEMP");
				if (bcomp.temp == 0xFFFF) {
					_sprintf(str, "--°C");
				} else {
					_sprintf(str, "%d.%d°C", (int)bcomp.temp/10, (int)(bcomp.temp%10));
				}
				if (bcomp.page & GUI_FLAG_GRAPH) {
					graph_puts16(64, 16, 1, str);
					diagram_draw(&bcomp.dia_temp);
				} else {
					graph_puts32c(64, 24, str);
				}
				break;
            case 4:
				// -----------------------------------------------------------------
				// Temp
				// -----------------------------------------------------------------
				graph_puts16(64, 0, 1, "PRESS");
				if (isnan(bcomp.mmhg)) {
					_sprintf(str, "--mmHg");
				} else {
					_sprintf(str, "%dmmHg", (int)bcomp.mmhg);
				}
                graph_puts32c(64, 24, str);
				break;                
            default:
                DBG("unknown page (%d)\r\n", bcomp.page);
                if (bcomp.page > 0 && bcomp.page < 5) {
                    if (buttons & BUTT_SW2) {
                        BCOMP_PAGE_PREV();
                    } else {
                        BCOMP_PAGE_NEXT();
                    }
                } else {
                    if (buttons & BUTT_SW2) {
                        bcomp.page = 4;
                    } else {
                        bcomp.page = 1;
                    }
                }
                goto repeate;
            }
        }
        // -----------------------------------------------------------------
        // Обновление экрана:
        // -----------------------------------------------------------------
#ifdef _DBGOUT
        ms = get_ms_timer(); 
#endif //_DBGOUT
        i2c_busy = 1;
        graph_update(); 
        i2c_busy = 0;
#ifdef _DBGOUT
        ms = get_ms_timer() - ms;
        DBG("graph_update() work %dms\r\n", ms);
#endif //_DBGOUT
#endif
    }
    return 0;
}

#if defined( WIN32 )
// Точка входа для отладочной WIN-версии:
int main(int argc, char **argv) {
	uint32_t addr;
	
	printf("-----------------------------------------------------------\r\n");
	printf("BCOMP11 Win32 PC build\r\n");
	printf("-----------------------------------------------------------\r\n");
	printf("A (or mouse click) - next, S - previous\r\n");
	printf("A long press - Enter, S long press - Cancel\r\n");
	printf("-----------------------------------------------------------\r\n");
	printf("\r\n");

	return lcd_init(ProcMain, "OLED", SIZE_X, SIZE_Y);
}
#endif
