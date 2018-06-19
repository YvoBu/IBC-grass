#include <time.h>
#include "timing.h"
CTiming gTimer;

CTiming::CTiming()
{

}

void CTiming::Start() {
    struct timespec start;

    clock_gettime (CLOCK_MONOTONIC_RAW , &start);
    u_start=(start.tv_sec*1000000ull)+(start.tv_nsec/1000ull);
}

int64_t CTiming::End() {
    struct timespec end;

    clock_gettime (CLOCK_MONOTONIC_RAW , &end);
    u_end=(end.tv_sec*1000000ull)+(end.tv_nsec/1000ull);
    return (u_end-u_start);
}
