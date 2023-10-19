#include <stdint.h>
#include <string.h>

/*
    Name  : CRC-8
    Poly  : 0x31    x^8 + x^5 + x^4 + 1
    Init  : 0xFF
    Revert: false
    XorOut: 0x00
    Check : 0xF7 ("123456789")
    MaxLen: 15 байт(127 бит) - обнаружение
        одинарных, двойных, тройных и всех нечетных ошибок
*/
uint8_t crc8(uint8_t *block, size_t len) {
    uint8_t crc = 0xFF;
    while (len--) {
        crc ^= *block++;
        for (int i = 0; i < 8; i++) {
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
        }
    }
    return crc;
}
