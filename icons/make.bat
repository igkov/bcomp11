@ECHO OFF
@ECHO process...
@echo.#include ^<stdint.h^> > icons11.c
@echo.#include ^"icons.h^" >> icons11.c
@echo.>> icons11.c
img2c.exe ico16-100kms.bmp ico16_100kms s >> icons11.c
img2c.exe ico16-at.bmp ico16_at s >> icons11.c
img2c.exe ico16-battery.bmp ico16_battery s >> icons11.c
img2c.exe ico16-engine.bmp ico16_engine s >> icons11.c
img2c.exe ico16-fuel.bmp ico16_fuel s >> icons11.c
img2c.exe ico16-mt.bmp ico16_mt s >> icons11.c
img2c.exe ico16-km.bmp ico16_km s >> icons11.c
img2c.exe ico16-kmh.bmp ico16_kmh s >> icons11.c
img2c.exe ico16-road.bmp ico16_road s >> icons11.c
img2c.exe ico16-rpm.bmp ico16_rpm s >> icons11.c
img2c.exe ico16-time.bmp ico16_time s >> icons11.c
img2c.exe ico16-temp.bmp ico16_temp s >> icons11.c
img2c.exe ico16-warning.bmp ico16_warning s >> icons11.c
@echo.>> icons11.c
img2c.exe ico64-mitsu.bmp ico64_mitsu ps >> icons11.c
img2c.exe ico64-nissan.bmp ico64_nissan ps >> icons11.c
@echo.>> icons11.c
img2c.exe ico48-engine.bmp ico48_engine ps >> icons11.c
img2c.exe ico48-silkroad.bmp ico48_silkroad ps >> icons11.c
img2c.exe ico48-temp.bmp ico48_temp ps >> icons11.c
img2c.exe ico48-battery.bmp ico48_battery ps >> icons11.c
img2c.exe ico48-trans.bmp ico48_trans ps >> icons11.c
img2c.exe ico48-connect.bmp ico48_connect ps >> icons11.c
img2c.exe ico48-gps.bmp ico48_gps ps >> icons11.c
img2c.exe ico48-service2.bmp ico48_service ps >> icons11.c
@echo.>> icons11.c
img2c.exe ico32-at-p.bmp ico32_at_p ps >> icons11.c
img2c.exe ico32-at-n.bmp ico32_at_n ps >> icons11.c
img2c.exe ico32-at-r.bmp ico32_at_r ps >> icons11.c
img2c.exe ico32-at-d1.bmp ico32_at_d1 ps >> icons11.c
img2c.exe ico32-at-d2.bmp ico32_at_d2 ps >> icons11.c
img2c.exe ico32-at-d3.bmp ico32_at_d3 ps >> icons11.c
img2c.exe ico32-at-d4.bmp ico32_at_d4 ps >> icons11.c
img2c.exe ico32-at-d5.bmp ico32_at_d5 ps >> icons11.c
