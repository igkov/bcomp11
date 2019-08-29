#include "moon.h"

FLOAT moon_phase(FLOAT jd) {
    FLOAT jdi = 2451550.1f; // 14:24 6-01-2000
    return fracf((jd-jdi) / 29.530588853f);
}
