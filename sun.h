#ifndef __SUN_H__
#define __SUN_H__

void suncalc(
    int myYear, int myMonth, int myDay, 
    int myHour, int myMinute, int mySeconds, 
    int offset, 
    float lat, float lon,
    double *rise, double *set, double *day);

#endif // __SUN_H__
