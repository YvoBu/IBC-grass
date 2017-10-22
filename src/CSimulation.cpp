#include <iostream>

#include "CThread.h"
#include "itv_mode.h"
#include "Grid.h"
#include "Output.h"
#include "GridEnvir.h"
#include "Parameters.h"
#include "CSimulation.h"
#include "IBC-grass.h"

CSimulation::CSimulation(int i, std::string aConfig)
{
    Configuration = aConfig;
    RunNr         = i;
}

bool CSimulation::InitInstance() {
    GetSim(Configuration);

    std::cout << getSimID() << std::endl;
    std::cout << "Run " << RunNr << " \n";

    InitRun();

    return true;
}

int CSimulation::Run() {
    OneRun();
    return 0;
}

void CSimulation::ExitInstance() {
    ThreadDone();
}
