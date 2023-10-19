/*
    virtuino.c
    
    Прослойка-драйвер для работы с приложением Virtuino на Андроиде. 
    Предполагается его использование вместо драйвера elog.c.
    Модули Bluetooth могут использоваться любые, которые обеспечивают 
    профиль SPP и UART-интерфейс (к примеру, RN-42 или HC-05).
    
    igorkov.org / Igor Kovalenko / 2018
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "virtuino.h"
#include "bcomp.h"
#include "uart0.h"
#include "dbg.h"

//
// Структура-описание данных на отправку:
//
const static virtuino_unit_t units[] = {
    // id,       type,                                 vtype,res, value pointer
    {   1,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.time },
    {   2,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.speed },
    {   3,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.rpm },
    {   4,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.t_engine },
    {   5,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.t_akpp },
    {   6,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.t_ext },
    {   7,  VIRTUINO_TYPE_FLOAT | VIRTUINO_ACCESS_READ,  'V', 0, &bcomp.v_ecu },
    {   8,  VIRTUINO_TYPE_BYTE | VIRTUINO_ACCESS_READ,   'V', 0, &bcomp.at_drive },
    {   9,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.p_intake },
    {  10,  VIRTUINO_TYPE_INT | VIRTUINO_ACCESS_READ,    'V', 0, &bcomp.p_fuel },
    {  11,  VIRTUINO_TYPE_FLOAT | VIRTUINO_ACCESS_READ,  'V', 0, &bcomp.fuel_level },
    {  12,  VIRTUINO_TYPE_DOUBLE | VIRTUINO_ACCESS_READ, 'V', 0, &bcomp.fuel },
    {  13,  VIRTUINO_TYPE_DOUBLE | VIRTUINO_ACCESS_READ, 'V', 0, &bcomp.dist },

    {   1,  VIRTUINO_TYPE_BYTE | VIRTUINO_ACCESS_READ | VIRTUINO_ACCESS_WRITE,  'Q', 0, &bcomp.service },

    {   1,  VIRTUINO_TYPE_STRING | VIRTUINO_ACCESS_READ,'T', 0, &bcomp.vin },
};

static void virtuino_unit_get(const virtuino_unit_t *punit, char *strout) {
    switch (punit->type & VIRTUINO_TYPE_MASK) {
    case VIRTUINO_TYPE_INT:
        _sprintf(strout, "!%c%02d=%d$", punit->vtype, punit->id, *(int*)punit->value);
        break;
    case VIRTUINO_TYPE_BYTE:
        _sprintf(strout, "!%c%02d=%d$", punit->vtype, punit->id, *(char*)punit->value);
        break;
    case VIRTUINO_TYPE_FLOAT:
        if (isnan(*(float*)punit->value)) {
            _sprintf(strout, "!%c%02d=NAN$", punit->vtype, punit->id);
        } else {
            _sprintf(strout, "!%c%02d=%d.%03d$", punit->vtype, punit->id, (int)*(float*)punit->value, 
                (int)(*(float*)punit->value * 1000) % 1000);
        }
        break;
    case VIRTUINO_TYPE_DOUBLE:
        if (isnan(*(double*)punit->value)) {
            _sprintf(strout, "!%c%02d=NAN$", punit->id);
        } else {
            _sprintf(strout, "!%c%02d=%d.%03d$", punit->vtype, punit->id, (int)*(double*)punit->value, 
                (int)(*(double*)punit->value * 1000) % 1000);
        }
        break;
    case VIRTUINO_TYPE_STRING:
        _sprintf(strout, "!%c%02d=%s$", punit->vtype, punit->id, *(char**)punit->value);
        break;
    default:
        DBG("virtuino_unit_get(): unknown value type (%d)!\r\n", punit->type & VIRTUINO_TYPE_MASK);
    }
}

// Не используется реализация atof, т.к. она подключает достаточно много
// паразитного кода (порядка 2кб). Эта автономная реализация намного легче.
static double stof(const char* s){
    double rez = 0;
    double fact = 1;
    int p = 0; // point seen flag
    if (*s == '-') {
        s++;
        fact = -1;
    }
    for ( ; *s; s++) {
        int d;
        if (*s == '.' || *s == ',') {
            p = 1; 
            continue;
        }
        d = *s - '0';
        if (d >= 0 && d <= 9) {
            if (p) {
                fact /= 10.0f;
            }
            rez = rez * 10.0f + (float)d;
        }
    }
    return rez * fact;
};

static void virtuino_unit_set(const virtuino_unit_t *punit, const char *str) {
    switch (punit->type & VIRTUINO_TYPE_MASK) {
    case VIRTUINO_TYPE_INT:
        *(int*)punit->value = (int)atoi(str);
        break;
    case VIRTUINO_TYPE_BYTE:
        *(char*)punit->value = (int)atoi(str);
        break;
    case VIRTUINO_TYPE_FLOAT:
        *(float*)punit->value = (float)stof(str);
        break;
    case VIRTUINO_TYPE_DOUBLE:
        *(double*)punit->value = (double)stof(str);
        break;
    case VIRTUINO_TYPE_STRING:        
        memset(*(char**)punit->value, 0, punit->len);
        if ((int)(strstr(str, "$") - str) < punit->len-1) {
            memcpy(*(char**)punit->value, str, (int)(strstr(str, "$") - str));
        }
    default:
        DBG("virtuino_unit_set(): unknown value type (%d)!\r\n", punit->type & VIRTUINO_TYPE_MASK);
    }
}

int virtuino_unit_find(uint8_t ch, uint8_t type) {
    int i;
    for (i=0; i<sizeof(units)/sizeof(virtuino_unit_t); i++) {
        if (units[i].id == ch && units[i].vtype == type) {
            return i;
        }
    }
    return -1;
}

#if 0
// !!! WARNING !!!
// Функция работает не в рамках протокола Virtuino.
// Virtuino делает запросы параметров через определенные промежутки времени.
// Здесь же реализована отправка данных самостоятельно каждую секунду.
// Таким же образом работает elog.c модуль.
// В процессе, происходит определение команды и производятся действия/ответы.
void virtuino_proc(void) {
    int i;
    int offset = 0;
    for (i=0; i<sizeof(units)/sizeof(virtuino_unit_t); i++) {
        virtuino_unit_get(&units[i], &virtuino_str[offset]);
        offset = strlen(virtuino_str);
    }
    // Выставляем данные на асинхронную отправку:
    uart0_puts(virtuino_str);    
}
#endif

void virtuino_proc(uint8_t data) {
    static int offset = 0;
    static char cmd[64];

    uint8_t ch;
    char *tmp;
    int unit_n;

#if defined( _DBGOUT )
    uart0_putchar(data);
#endif

    if (data == '!') {
        memset(cmd, 0, sizeof(cmd));
        offset = 0;
    }

    if (offset < sizeof(cmd)-1) {
        cmd[offset] = data;
        offset++;
    }
          
    if (data == '$') {
        ch = atoi(&cmd[2]);
        DBG("virtuino_unit_find(%d, '%c')\r\n", ch, cmd[1]);
        unit_n = virtuino_unit_find(ch, cmd[1]);
        if (cmd[1] != 'C' && unit_n == -1) {
            // TODO: set error
            goto end;
        }
        tmp = strstr(cmd, "=");
        // !!! PROC !!!
        switch (cmd[1]) {
        case 'C': // Команда инициализации
            strcpy(cmd, VIRTUINO_VERSION_ANSWER);
            break;
        case 'O':
        case 'I':
            DBG("WARNING: cmd = '%s'\r\n", cmd);
            break;
        case 'A': // Аналоговый вход
        case 'D': // Виртуальный вывод VD
        case 'V': // Виртуальный вывод V
        case 'Q': // Цифровой вывод
        case 'T': // Строка на выход
            if (tmp[1] == '?') {
                if (units[unit_n].type & VIRTUINO_ACCESS_READ) {
                    virtuino_unit_get(&units[unit_n], cmd);
                } else {
                    // TODO: set error
                }
            } else {
                if (units[unit_n].type & VIRTUINO_ACCESS_WRITE) {
                    virtuino_unit_set(&units[unit_n], &tmp[1]);
                } else {
                    // TODO: set error
                }
            }
            break;
        default:
            DBG("virtuino_proc(): unknown command '%c'\r\n", cmd[1]);
        }
        // Выставляем данные на асинхронную отправку:
        uart0_puts(cmd);
    }
end:
    return;
}

