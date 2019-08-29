#ifndef __SUN_H__
#define __SUN_H__

#if 1
#define FLOAT float
#else
#define FLOAT double
#endif

FLOAT JulDay(int date, int month, int year, FLOAT UT);

void suncalc(
    int myYear, int myMonth, int myDay, 
    int myHour, int myMinute, int mySeconds, 
    int offset, 
    FLOAT lat, FLOAT lon,
    FLOAT *rise, FLOAT *set, FLOAT *day);

FLOAT fracf(FLOAT X);

#endif // __SUN_H__
