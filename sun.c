#include <stdio.h> 
#include <stdbool.h> 
#include <math.h> 

#include "dbg.h"

/*
    Широта, Latitude - идет с севера (North, N) на юг (South, S).
    Долгота, Longitude - идет с запада (West, W) на восток (East, E).
    
    Практически всегда координаты идут в порядке “широта, долгота” - Lat, Lng. 
    В Google Maps, в Яндекс-Картах, в Википедии. Это стоит зазубрить: в географии сначала идет Y, потом X.
 */

#define PI 3.141592654 // Pi

static double JulDay(int date, int month, int year, double UT){
    double A, B;
    if (year < 1900) {
        year = year + 1900;
    }
    if (month <= 2) {
        month = month + 12; 
        year = year - 1;
    }
    B = floor(year/400.0) - floor(year/100.0)  + floor(year/4.0);
    A = 365.0*year - 679004.0;
    return A + B + floor(30.6001 * (month + 1)) + date + UT/24.0 + 2400000.5;
}

static double sunL(double T) {
    int L = 280.46645 + 36000.76983*T + 0.0003032*T*T;
    L = L % 360;
    if (L < 0) {
        L = L + 360;
    }
    return L;
}

static double EPS(double T) {
    double K = PI/180.0;
    double LS = sunL(T);
    double LM = 218.3165 + 481267.8813*T;
    double eps0 =  23.0 + 26.0/60.0 + 21.448/3600.0 - (46.8150*T + 0.00059*T*T - 0.001813*T*T*T)/3600;
    double omega = 125.04452 - 1934.136261*T + 0.0020708*T*T + T*T*T/450000;
    double deltaEps = (9.20*cos(K*omega) + 0.57*cos(K*2*LS) + 0.10*cos(K*2*LM) - 0.09*cos(K*2*omega))/3600;
    return eps0 + deltaEps;
}

static double deltaPSI(double T) {
    double K = PI/180.0;
    double deltaPsi, omega, LS, LM;
    LS = sunL(T);
    LM = 218.3165 + 481267.8813*T;
    LM = (int)LM % 360;
    if (LM < 0) {
        LM = LM + 360;
    }
    omega = 125.04452 - 1934.136261*T + 0.0020708*T*T + T*T*T/450000;
    deltaPsi = -17.2*sin(K*omega) - 1.32*sin(K*2*LS) - 0.23*sin(K*2*LM) + 0.21*sin(K*2*omega);
    deltaPsi = deltaPsi / 3600.0;
    return deltaPsi;
}

#if 0
static double DEC;
static double RA;

static double RightAscension(double T) {
    double K = PI/180.0;
    double L, M, C, lambda,  eps, delta, theta;
    L = sunL(T);
    M = 357.52910 + 35999.05030*T - 0.0001559*T*T - 0.00000048*T*T*T;
    M = (int)M % 360;
    if (M < 0) M = M + 360;
    C = (1.914600 - 0.004817*T - 0.000014*T*T)*sin(K*M);
    C = C + (0.019993 - 0.000101*T)*sin(K*2*M);
    C = C + 0.000290*sin(K*3*M);
    theta = L + C; // true longitude of the Sun
    eps = EPS(T);
    eps = eps + 0.00256*cos(K*(125.04 - 1934.136*T));
    lambda = theta - 0.00569 - 0.00478*sin(K*(125.04 - 1934.136*T)); // apparent longitude of the Sun
    RA = atan2(cos(K*eps)*sin(K*lambda), cos(K*lambda));
    RA = RA/K;
    if (RA < 0) RA = RA + 360.0;
    delta = asin(sin(K*eps)*sin(K*lambda));
    delta = delta/K;
    DEC = delta;
    DBG("DEC = %f\r\n", round(1000*DEC)/1000);
    return RA;
}

static double eot(double jd) { 
    double K = PI/180.0;
    double T = (jd - 2451545.0) / 36525.0;
    double eps = EPS(T);
    double RA = RightAscension(T);
    double LS = sunL(T);
    double deltaPsi = deltaPSI(T);
    double E = LS - 0.0057183 - RA + deltaPsi * cos(K * eps);
    if (E > 5) {
        E = E - 360.0;
    }
    E = E * 4; // deg. to min
    E = round(1000 * E) / 1000;
    return E;
}
#endif

static double computeGHA (int T, int M, int J, double STD) {
    double K = PI/180.0, N, X, XX, P;	
    N = 365 * J + T + 31 * M - 46; 
    if (M < 3) {
        N = N + floor((J-1)/4);
    } else {
        N = N - floor(0.4*M + 2.3) + floor(J/4);
    }
    P = STD / 24.0;
    X = (P + N - 7.22449E5) * 0.98564734 + 279.306;
    X = X * K;
    XX = -104.55 * sin(X) - 429.266 * cos(X) + 595.63 * sin(2.0 * X) - 2.283 * cos(2.0 * X);
    XX = XX + 4.6 * sin(3.0 * X) + 18.7333 * cos(3.0 * X);
    XX = XX - 13.2 * sin(4.0 * X) - cos(5.0 * X) - sin(5.0 * X) / 3.0 + 0.5 * sin(6.0 * X) + 0.231;
    XX = XX / 240.0 + 360.0 * (P + 0.5);
    if (XX > 360.0) {
        XX = XX - 360.0;
    }
    return XX;
}

static double computeHeight(double dec, double latitude, double longit, double gha) {
    double K = PI/180.0;
    double lat_K = latitude*K;
    double dec_K = dec*K;
    double x = gha + longit;
    double sinHeight = sin(dec_K)*sin(lat_K) + cos(dec_K)*cos(lat_K)*cos(K*x);	
    return asin(sinHeight) / K;
}

static double frac(double X) {
    X = X - floor(X);
    if (X < 0) {
        X = X + 1.0;
    }
    return X;
}

static double sunDecRA (int what, double jd) {
    double PI2 = 2.0 * PI;
    double cos_eps = 0.917482;
    double sin_eps = 0.397778;
    double M, DL, L, SL, X, Y, Z, R;
    double T, dec;
    //double ra;
    T = (jd - 2451545.0) / 36525.0;	// number of Julian centuries since Jan 1, 2000, 0 GMT
    M = PI2*frac(0.993133 + 99.997361*T);
    DL = 6893.0*sin(M) + 72.0*sin(2.0*M);
    L = PI2*frac(0.7859453 + M/PI2 + (6191.2*T+DL)/1296000);
    SL = sin(L);
    X = cos(L);
    Y = cos_eps*SL;
    Z = sin_eps*SL;
    R = sqrt(1.0-Z*Z);
    dec = (360.0/PI2)*atan(Z/R);
    //ra = (48.0/PI2)*atan(Y/(X+R));
    //if (ra < 0) {
    //    ra = ra + 24.0;
    //}
    //if (what == 1) 
        return dec; 
    //else 
    //    return ra;
}

#if 0
static double zero1;
static double zero2;
static double YE;

static int QUAD(double yMinus, double yPlus, double Y0) {
    int NZ = 0;
    double A = 0.5*(yMinus+yPlus) - Y0;
    double B = 0.5*(yPlus-yMinus);
    double C = Y0;
    double XE = -B/(2.0*A);
    YE = (A*XE+B)*XE + C;
    double DIS = B*B - 4.0*A*C;
    if (DIS >= 0) {
        double DX = 0.5*sqrt(DIS) / fabs(A);
        zero1 = XE - DX;
        zero2 = XE + DX;
        if (fabs(zero1) <= 1.0) {
            NZ = NZ + 1;
        }
        if (fabs(zero2) <= 1.0) {
            NZ = NZ + 1;
        }
        if (zero1 < -1.0) {
            zero1 = zero2;
        }
    }
    return NZ;
}
#endif

#define FLAG_RISE  0x01
#define FLAG_SETT  0x02
#define FLAG_ABOVE 0x04

static int riseset(int DATE, int MONTH, int YEAR, int HOUR, float lat, float lon, double *UTRISE, double *UTSET) {
    int ret = 0;

    double K  = PI/180.0;
    double sh = sin(-K*0.8333);
    double jd = JulDay (DATE, MONTH, YEAR, HOUR);
    double dec = sunDecRA(1,jd);
    //double ra = sunDecRA(2,jd);
    double gha = computeGHA (DATE, MONTH, YEAR, HOUR);
    double Y0 = sin(K*computeHeight(dec, lat, lon, gha)) - sh;
    double jdPlus = JulDay(DATE, MONTH, YEAR, HOUR+1.0);
    dec = sunDecRA(1,jdPlus);
    //ra  = sunDecRA(2,jdPlus);
    gha = computeGHA (DATE, MONTH, YEAR, HOUR+1.0);
    double yPlus = sin(K*computeHeight(dec, lat, lon, gha)) - sh;
    double jdMinus = JulDay(DATE, MONTH, YEAR, HOUR-1.0);
    dec = sunDecRA(1,jdMinus);
    //ra  = sunDecRA(2,jdMinus);
    gha = computeGHA (DATE, MONTH, YEAR, HOUR-1.0);
    double yMinus = sin(K*computeHeight(dec, lat, lon, gha)) - sh;
    //ABOVE = (yMinus > 0.0);
    if (yMinus > 0.0) {
        ret |= FLAG_ABOVE; 
    }
#if 0
    int NZ = QUAD(yMinus, yPlus, Y0);
#else
    double zero1;
    double zero2;
    int NZ = 0;
    double A = 0.5*(yMinus+yPlus) - Y0;
    double B = 0.5*(yPlus-yMinus);
    double C = Y0;
    double XE = -B/(2.0*A);
    double YE = (A*XE+B)*XE + C;
    double DIS = B*B - 4.0*A*C;
    if (DIS >= 0) {
        double DX = 0.5*sqrt(DIS) / fabs(A);
        zero1 = XE - DX;
        zero2 = XE + DX;
        if (fabs(zero1) <= 1.0) {
            NZ = NZ + 1;
        }
        if (fabs(zero2) <= 1.0) {
            NZ = NZ + 1;
        }
        if (zero1 < -1.0) {
            zero1 = zero2;
        }
    }
#endif
    switch (NZ) {
    case 0: 
        break;
    case 1:
        if (yMinus < 0.0) {
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
        if (YE < 0.0) {
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
    float lat, float lon,
    double *rise, double *set, double *day
    ) {
    double UT;
    double UTRISE;
    double UTSET;
    int flags = 0;
   
    DBG("TIME: %02d:%02d:%02d\r\n", myHour, myMinute, mySeconds);
    DBG("DATE: %02d-%02d-%04d\r\n", myDay, myMonth, myYear);
    DBG("GPS:  %f %f\r\n", lat, lon);
    
    if (offset*60 >= 1320) {
        offset = offset - 1440/60;
    }
    //UT = myHour + (myMinute - offset*60)/60.0 + mySeconds/3600.0;
    UT = myHour + myMinute/60.0 + mySeconds/3600.0;
    
#if 0
    double EOT;
    double JD;

    JD = JulDay(myDay, myMonth, myYear, UT);
    DBG("JD = %f\r\n", JD);
    EOT = eot(JD);
    
    double GHA  = computeGHA(myDay, myMonth, myYear, UT);
    double elev = computeHeight(DEC, lat, lon, GHA);
    elev = round(elev*100)/100;
    DBG("ELEV = %f\r\n", elev);
#endif

    for (int i = -offset; i < -offset+24; i++) {
        flags |= riseset(myDay, myMonth, myYear, i, lat, lon, &UTRISE, &UTSET);
        //DBG("i = %d flags = %x\r\n", i, flags);
        if ((flags & FLAG_RISE) && (flags & FLAG_SETT)) {
            break;
        }
    }
    
    //double rise;
    //double set;
    //double day;
    
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
        DBG("SUN RISE: %2d:%02d (%f)\r\n", (int)*rise, (int)(frac(*rise) * 60), *rise);
    } else {
        if (flags & FLAG_ABOVE) {
            DBG("SUN RISE: --:-- (DAY)\r\n");
        } else {
            DBG("SUN RISE: --:-- (NIGHT)\r\n");
        }
        *rise = NAN;
    }

    if (flags & FLAG_SETT) {
        DBG("SUN SET:  %2d:%02d (%f)\r\n", (int)*set, (int)(frac(*set) * 60), *set);
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
    DBG("LENGTH OF DAY: %2d:%02d (%f)\r\n", (int)*day, (int)(frac(*day) * 60), *day);
}

#if defined( SUN_MAIN )
void main() {
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
