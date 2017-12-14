#include "leds.h"
#include "dbg.h"

void leds_init(void) {
}

void led_red(int state) {
	DBG("led_red(%d)\r\n", state);
}

void led_green(int state) {
	DBG("led_green(%d)\r\n", state);
}
