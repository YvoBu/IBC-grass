#ifndef CSIMULATION_H
#define CSIMULATION_H

#include <string>
#include "RandomGenerator.h"

class CSimulation : public GridEnvir
{
public:
    CSimulation(int i, std::string aConfig);
    virtual bool InitInstance();
    virtual int Run(void);
    virtual void ExitInstance();
private:
    std::string Configuration;
public:
//    RandomGenerator rng;
};

#endif // CSIMULATION_H
