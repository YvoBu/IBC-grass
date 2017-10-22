#ifndef CSIMULATION_H
#define CSIMULATION_H

#include <string>

class CSimulation : public GridEnvir
{
public:
    CSimulation(int i, std::string aConfig);
    virtual bool InitInstance();
    virtual int Run(void);
    virtual void ExitInstance();
private:
    int         RunNr;
    std::string Configuration;
};

#endif // CSIMULATION_H
