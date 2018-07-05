#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include "bcomp.h"

// Поддержка длинных нажатий.  Функция длинных нажатий нужна для управления 
// одной кнопкой и более удобного управления 2мя кнопками.
#define BUTTONS_SUPPORT_LONG 1

#if !defined( BUTTONS_ANALOG )
// Поддержка внешнего блока кнопок. Данный блок кнопок аналоговый, основан 
// на резистивных делителях.
#define BUTTONS_ANALOG 0
#endif

// Количество поддерживаемых кнопок:
#define MAX_BUTT 2

#if ( BUTTONS_SUPPORT_LONG )
#define BUTT_SW1       (1<<0)
#define BUTT_SW1_LONG  (1<<1)
#define BUTT_SW2       (1<<2)
#define BUTT_SW2_LONG  (1<<3)
#else
#define BUTT_SW1  (1<<0)
#define BUTT_SW2  (1<<1)
#endif

void button_init(void);
int button_read(void);

#endif // __BUTTONS_H__
