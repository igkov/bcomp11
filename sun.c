#include <stdio.h> 
#include <stdbool.h> 
#include <math.h> 

#include "dbg.h"
#include "sun.h"

/*
    Широта, Latitude - идет с севера (North, N) на юг (South, S).
    Долгота, Longitude - идет с запада (West, W) на восток (East, E).
    
    Практически всегда координаты идут в порядке “широта, долгота” - Lat, Lng. 
    В Google Maps, в Яндекс-Картах, в Википедии. Это стоит зазубрить: в географии сначала идет Y, потом X.
 */

#define PI    3.141592654f
#define PI2  (2*PI)

FLOAT JulDay(int date, int month, int year, FLOAT UT) {
    FLOAT A, B;
    if (year < 1900) {
        year = year + 1900;
    }
    if (month <= 2) {
        month = month + 12; 
        year = year - 1;
    }
    B = floorf(year/400.0f) - floorf(year/100.0f)  + floorf(year/4.0f);
    A = 365.0f*year - 679004.0f;
    return A + B + floorf(30.6001f * (month + 1)) + date + UT/24.0f + 2400000.5f;
}

static FLOAT computeGHA(int T, int M, int J, FLOAT STD) {
    FLOAT K = PI/180.0;
    FLOAT N;
    FLOAT X;
    FLOAT XX;
    FLOAT P;
    N = 365 * J + T + 31 * M - 46; 
    if (M < 3) {
        N = N + floorf((J-1)/4);
    } else {
        N = N - floorf(0.4f*M + 2.3f) + floorf(J/4);
    }
    P = STD / 24.0f;
    X = (P + N - 7.22449E5f) * 0.98564734f + 279.306f;
    X = X * K;
    XX = -104.55f * sinf(X) - 429.266f * cosf(X) + 595.63f * sinf(2.0f * X) - 2.283f * cosf(2.0f * X);
    XX = XX + 4.6f * sinf(3.0f * X) + 18.7333f * cosf(3.0f * X);
    XX = XX - 13.2f * sinf(4.0f * X) - cosf(5.0f * X) - sinf(5.0f * X) / 3.0f + 0.5f * sinf(6.0f * X) + 0.231f;
    XX = XX / 240.0f + 360.0f * (P + 0.5f);
    if (XX > 360.0f) {
        XX = XX - 360.0f;
    }
    return XX;
}

static FLOAT computeHeight(FLOAT dec, FLOAT latitude, FLOAT longit, FLOAT gha) {
    FLOAT K = PI/180.0;
    //FLOAT lat_K = latitude*K;
    //FLOAT dec_K = dec*K;
    FLOAT x = gha + longit;
    FLOAT sinHeight = sinf(dec*K)*sinf(latitude*K) + cosf(dec*K)*cosf(latitude*K)*cosf(K*x);	
    return asinf(sinHeight) / K;
}

FLOAT fracf(FLOAT X) {
    X = X - floorf(X);
    if (X < 0) {
        X = X + 1.0f;
    }
    return X;
}

static FLOAT sunDecRA(int what, FLOAT jd) {
    FLOAT sin_eps = 0.397778f;
    FLOAT M, DL, L, SL, Z, R;
    FLOAT T;
    T = (jd - 2451545.0f) / 36525.0f;	// number of Julian centuries since Jan 1, 2000, 0 GMT
    M = PI2 * fracf(0.993133f + 99.997361f*T);
    DL = 6893.0f*sinf(M) + 72.0f*sinf(2.0f*M);
    L = PI2*fracf(0.7859453f + M/PI2 + (6191.2f*T+DL)/1296000);
    SL = sinf(L);
    Z = sin_eps*SL;
    R = sqrtf(1.0f-Z*Z);
    return (360.0f/PI2) * atanf(Z/R);
}

#define FLAG_RISE  0x01
#define FLAG_SETT  0x02
#define FLAG_ABOVE 0x04

static int riseset(int DATE, int MONTH, int YEAR, int HOUR, float lat, float lon, FLOAT *UTRISE, FLOAT *UTSET) {
    int ret = 0;
    int NZ = 0;
    FLOAT zero1;
    FLOAT zero2;
    FLOAT YE;
    FLOAT K  = PI/180.0;
    FLOAT sh = sinf(-K*0.8333f);
    FLOAT jd = JulDay (DATE, MONTH, YEAR, HOUR);
    FLOAT dec = sunDecRA(1,jd);
    FLOAT gha = computeGHA (DATE, MONTH, YEAR, HOUR);
    FLOAT Y0 = sinf(K*computeHeight(dec, lat, lon, gha)) - sh;
    FLOAT jdPlus = JulDay(DATE, MONTH, YEAR, HOUR+1);
    FLOAT yPlus, jdMinus, yMinus;
    
    dec = sunDecRA(1,jdPlus);
    gha = computeGHA (DATE, MONTH, YEAR, HOUR+1);
    yPlus = sinf(K*computeHeight(dec, lat, lon, gha)) - sh;
    
    jdMinus = JulDay(DATE, MONTH, YEAR, HOUR-1);
    dec = sunDecRA(1,jdMinus);
    gha = computeGHA (DATE, MONTH, YEAR, HOUR-1);
    yMinus = sinf(K*computeHeight(dec, lat, lon, gha)) - sh;
    
    if (yMinus > 0.0) {
        ret |= FLAG_ABOVE; 
    }
    if (1) {
        FLOAT A = 0.5f*(yMinus+yPlus) - Y0;
        FLOAT B = 0.5f*(yPlus-yMinus);
        FLOAT C = Y0;
        FLOAT XE = -B/(2.0f*A);
        FLOAT DIS = B*B - 4.0f*A*C;
        YE = (A*XE+B)*XE + C;
        if (DIS >= 0.0f) {
            FLOAT DX = 0.5f*sqrtf(DIS) / fabsf(A);
            zero1 = XE - DX;
            zero2 = XE + DX;
            if (fabsf(zero1) <= 1.0f) {
                NZ = NZ + 1;
            }
            if (fabsf(zero2) <= 1.0f) {
                NZ = NZ + 1;
            }   
            if (zero1 < -1.0f) {
                zero1 = zero2;
            }
        }
    }
    switch (NZ) {
    case 0: 
        break;
    case 1:
        if (yMinus < 0.0f) {
            *UTRISE = HOUR + zero1; 
            //RISE   = true;
            ret |= FLAG_RISE; 
        } else {
            *UTSET = HOUR + zero1; 
            //SETT  = true;
            ret |= FLAG_SETT; 
        }
        break;
    case 2:
        if (YE < 0.0f) {
            *UTRISE = HOUR + zero2; 
            *UTSET  = HOUR + zero1;
        } else {
            *UTRISE = HOUR + zero1; 
            *UTSET  = HOUR + zero2;
        }
        //RISE = true;
        ret |= FLAG_RISE; 
        //SETT = true;
        ret |= FLAG_SETT; 
        break;
    }
    return ret;
}

void suncalc(
        int myYear, int myMonth, int myDay, 
        int myHour, int myMinute, int mySeconds, 
        int offset, 
        FLOAT lat, FLOAT lon,
        FLOAT *rise, FLOAT *set, FLOAT *day
    ) 
{
    FLOAT UTRISE;
    FLOAT UTSET;
    int flags = 0;
    int i;
   
    DBG("TIME: %02d:%02d:%02d\r\n", myHour, myMinute, mySeconds);
    DBG("DATE: %02d-%02d-%04d\r\n", myDay, myMonth, myYear);
    DBG("GPS:  %f %f\r\n", lat, lon);
    
    if (offset*60 >= 1320) {
        offset = offset - 1440/60;
    }
    
    for (i = -offset; i < -offset+24; i++) {
        flags |= riseset(myDay, myMonth, myYear, i, lat, lon, &UTRISE, &UTSET);
        if ((flags & FLAG_RISE) && (flags & FLAG_SETT)) {
            break;
        }
    }
    
    if ((flags & FLAG_RISE) || (flags & FLAG_SETT)) {
        if (flags & FLAG_RISE) {
            *rise = UTRISE + offset;
            if (*rise > 24) rise -= 24;
            if (*rise < 0)  rise += 24;
        }
        if (flags & FLAG_SETT) {
            *set = UTSET + offset;
            if (*set > 24) set -= 24;
            if (*set < 0)  set += 24;
        }
        *day = *set - *rise;
    }

    if (flags & FLAG_RISE) {
        DBG("SUN RISE: %2d:%02d (%f)\r\n", (int)*rise, (int)(fracf(*rise) * 60), *rise);
    } else {
        if (flags & FLAG_ABOVE) {
            DBG("SUN RISE: --:-- (DAY)\r\n");
        } else {
            DBG("SUN RISE: --:-- (NIGHT)\r\n");
        }
        *rise = NAN;
    }

    if (flags & FLAG_SETT) {
        DBG("SUN SET:  %2d:%02d (%f)\r\n", (int)*set, (int)(fracf(*set) * 60), *set);
    }
    else {
        if (flags & FLAG_ABOVE) {
            DBG("SUN SET:  --:-- (DAY)\r\n");
            *day = 24;
        } else {
            DBG("SUN SET:  --:-- (NIGHT)\r\n");
            *day = 0;
        }
        *set = NAN;
    }
    if (*day < 0.0f) {
        *day = 24.0 + *day;
    }
    DBG("LENGTH OF DAY: %2d:%02d (%f)\r\n", (int)*day, (int)(fracf(*day) * 60), *day);
}

#if defined( SUN_MAIN )
void main(void) {
    int myDay = 23;     // Day
    int myMonth = 8;    // Month
    int myYear = 2019;  // Year
    int myHour = 19;    // Hour
    int myMinute = 30;  // Minute
    int mySeconds = 0;  // Seconds

    int offset = 3;     // TimeZone

    float lat = 55.8f;
    float lon = 37.6f;

    // A
    suncalc(myYear, myMonth, myDay, myHour, myMinute, mySeconds, offset, lat, lon);
    DBG("\r\n");
    // BAD DAY LENGTH!!!!
    suncalc(2019,   5, 17, 12,  0,  0, 3, 69.951191, 31.925871);
    DBG("\r\n");
    // E
    suncalc(2018,  11, 25, 12,  0,  0, 3, 69.951191, 31.925871);
    DBG("\r\n");
    suncalc(2018,  11, 26, 12,  0,  0, 3, 69.951191, 31.925871);
    DBG("\r\n");
    suncalc(2019,   5, 16, 12,  0,  0, 3, 69.951191, 31.925871);
    DBG("\r\n");
    suncalc(2019,   5, 17, 12,  0,  0, 3, 69.951191, 31.925871);
    DBG("\r\n");
    suncalc(2019,   5, 18, 12,  0,  0, 3, 69.951191, 31.925871);
    DBG("\r\n");
}
#endif
