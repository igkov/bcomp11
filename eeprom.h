#ifndef __I2C_EEPROM__
#define __I2C_EEPROM__

// For 24LCxx:
#define EEPROM_STD_ADDRESS 0xA0

unsigned char ee_read(unsigned short addr, unsigned char *buff, unsigned char len);
unsigned char ee_write(unsigned short addr, unsigned char *buff, unsigned char len);
void ee_clear(void);

#endif // __I2C_EEPROM__
