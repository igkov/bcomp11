#include <bcomp.h>

const bcomp_config_t bconfig = {
	0xEAEAEAEA,            // begin label
	CONFIGURATON_VERSION,  // version
	500000,                // CAN speed
	115200,                // UART speed
#if ( INSIDE_VERSION == 1 )
	16,
#else
	#if ( VIRTUINO_SUPPORT == 1 )
		0,
	#else
		64,                    // EEPROM size in kBit
	#endif
#endif
	16,                    // contrast
	4,                     // start delay
	0,                     // start sound
	100,                   // t_engine_warning
	115,                   // t_akpp_warning
	14.5f,                 // v_max
	12.3f,                 // v_min
	3.33f,                 // fuel_coeff
	1.0f,                  // speed_coeff
	84000,                 // moto_dist_offset
	0,                     // moto_time_offset
	15000,                 // service distation (default = 15000km, recommended 7500km)
	250,                   // service moto time (default = 250h, recommended 150h)
#if ( INSIDE_VERSION == 1 )
	1,
#else
	#if ( VIRTUINO_SUPPORT == 1 )
		255,                   // 255 - no display
	#else
		0,                     // 0 - OLED SSD1306, 1 - OLED SH1106
	#endif
#endif
#if ( VIRTUINO_SUPPORT == 1 )
	0,                     // UART-logging off
#else
	1,                     // UART-logging on
#endif
	0,                   // res1
	0,                   // res2
#if ( INSIDE_VERSION == 1 )
	{ 3700.0f, 313.15f, 1000.0f },  // Mitsubishi NMPS termistor
#else
	{ 4300.0f, 298.15f, 10000.0f }, // EPCOS termistor
#endif
	0xAEAEAEAE,            // end label
};
