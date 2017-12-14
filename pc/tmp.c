
#if 0
	oled_init(0x80, 0);
	DBG("oled_init() ok!\r\n");
	while (1) {
		graph_clear();
		DBG("graph_clear() ok!\r\n");
		_sprintf(str, "(%d)!", i);
		graph_puts8(64,0,1,"Hello world!"); 
		graph_puts16(64,16+i%20,1,str); 
		DBG("graph_puts16(\"%s\") ok!\r\n", str);
		graph_update();
		DBG("graph_update() ok!\r\n");
		led_green(1);
		delay_ms(300);
		led_green(0);
		delay_ms(300);
		i++;
	}
#endif

#if 0
	while (1) {
		graph_clear();
		graph_pic(&ico64_mitsu, 0,0);
		graph_update();
		delay_ms(3000);
	}
#endif


#if 0
	oled_init(0x80, 0);
	DBG("oled_init() ok!\r\n");
	while (1) {
		graph_clear();
		DBG("graph_clear() ok!\r\n");
		_sprintf(str, "(%d)!", i);
		graph_puts8(64,0,1,"Hello world!"); 
		graph_puts16(64,16+i%20,1,str); 
		DBG("graph_puts16(\"%s\") ok!\r\n", str);
		graph_update();
		DBG("graph_update() ok!\r\n");
		led_green(1);
		delay_ms(300);
		led_green(0);
		delay_ms(300);
		i++;
	}
#endif

#if 0
	oled_init(0x10, 0);
	DBG("oled_init() ok!\r\n");
	while (1) {
		graph_clear();
		DBG("graph_clear() ok!\r\n");
		graph_puts32c(64,16,"12°C"); 
		DBG("graph_puts32c(\"%s\") ok!\r\n", str);
		graph_update();
		DBG("graph_update() ok!\r\n");
		delay_ms(300);
		//break;
	}
#endif

#if 0
	// Тест кнопок:
	while (1) {
		int ret, state;
		__WFI();
		// Состояние кнопок:
		state = button_read();
		if (state & BUTT_SW1) {
			DBG("BUTT_SW1\r\n");
		}
		if (state & BUTT_SW2) {
			DBG("BUTT_SW2\r\n");
		}
		if (state & BUTT_SW1_LONG) {
			DBG("BUTT_SW1 Long!\r\n");
		}
		if (state & BUTT_SW2_LONG) {
			DBG("BUTT_SW2 Long!\r\n");
		}
	}
#endif

#if 0
	// Дамп EEPROM:
	if (1) {
		int ret;
		int n, i;
		for (n = 0; n < 0x200; n += 16) {
			DBG("%04x: ", n);
			memset(str, 0xFF, 16);
			ret = ee_read(n, str, 16);
			if (ret) {
				DBG("ee_read return %d!\r\n", ret);
			}
			for (i = 0; i < 16; i++) {
				DBG("%02x ", str[i]);
			}
			DBG("\r\n");
		}
		while (1);
	}
#endif

#if 0
	// Тест EEPROM:
	if (1) {
		int ret;
		// EEPROM test:
		memset(str, 0xEE, 16);
		ret = ee_write(0x10, str, 16);
		DBG("ee_write() return %d\r\n", ret);
		memset(str, 0x00, 16);
		ret = ee_read(0x10, str, 16);
		DBG("ee_read() return %d\r\n", ret);
		DBG("Data: ");
		for (i=0; i<16; i++) {
			DBG("%02x ", str[i]);
		}
		DBG("\r\n");
		while (1);
	}
#endif
