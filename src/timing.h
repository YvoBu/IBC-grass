#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

class CTiming
{
public:
    CTiming();
    void Start();
    int64_t End();
private:
    int64_t         u_start;
    int64_t         u_end;
};

extern CTiming gTimer;

#endif // TIMING_H
