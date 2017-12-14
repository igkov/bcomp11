#include "bcomp.h"
#include "drive.h"
#include "graph.h"

#define AT_DRIVE_ICO 0

/* 
	show_drive()
	
	Вывод на экран активной передачи АКПП: 
 */
void show_drive(int x, int y) {
	if (bcomp.at_drive == 0x00) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_n,x-12,y);
#else
		graph_puts32c(x, y, "N");
#endif
	} else
	if (bcomp.at_drive == 0x01) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_d1,x-12,y);
#else
		graph_puts32c(x, y, "D/1");
#endif
	} else
	if (bcomp.at_drive == 0x02) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_d2,x-12,y);
#else
		graph_puts32c(x, y, "D/2");
#endif
	} else
	if (bcomp.at_drive == 0x03) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_d3,x-12,y);
#else
		graph_puts32c(x, y, "D/3");
#endif
	} else
	if (bcomp.at_drive == 0x04) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_d4,x-12,y);
#else
		graph_puts32c(x, y, "D/4");
#endif
	} else
	if (bcomp.at_drive == 0x05) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_d5,x-12,y);
#else
		graph_puts32c(x, y, "D/5");
#endif
	} else
	if (bcomp.at_drive == 0x0d) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_p,x-12,y);
#else
		graph_puts32c(x, y, "P");
#endif
	} else
	if (bcomp.at_drive == 0x0b) {
#if (AT_DRIVE_ICO == 1)
		graph_pic(&ico32_at_r,x-12,y);
#else
		graph_puts32c(x, y, "R");
#endif
	}
}
