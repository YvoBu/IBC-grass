
#include <iostream>
#include <string>
#include <sstream>

#include "itv_mode.h"
#include "Traits.h"
#include "Environment.h"
#include "RandomGenerator.h"
#include "Output.h"
#include "IBC-grass.h"

using namespace std;

const int Environment::WeeksPerYear = 30;
//-----------------------------------------------------------------------------

Environment::Environment()
{
	year = 1;
	week = 1;

	SimID = 0;
	ComNr = 0;
	RunNr = 0;
}

//-----------------------------------------------------------------------------

Environment::~Environment()
{
}

//-----------------------------------------------------------------------------

void Environment::GetSim(string data)
{
	/////////////////////////
	// Read in simulation parameters

	int IC_version;
	int mode;

	std::stringstream ss(data);

	ss	>> SimID 											// Simulation number
		>> ComNr 											// Community number
		>> IC_version 										// Stabilizing mechanisms
		>> mode												// (0) Community assembly (normal), (1) invasion criterion, (2) catastrophic disturbance
        >> ITVsd 						// Standard deviation of intraspecific variation
        >> Tmax 							// End of run year
        >> meanARes 						// Aboveground resources
        >> meanBRes 	 					// Belowground resources
        >> AbvGrazProb 					// Aboveground grazing: probability
        >> AbvPropRemoved 				// Aboveground grazing: proportion of biomass removed
        >> BelGrazProb 					// Belowground grazing: probability
        >> BelGrazPerc 					// Belowground grazing: proportion of biomass removed
        >> BelGrazAlpha					// For sensitivity analysis of Belowground Grazing algorithm
        >> BelGrazHistorySize			// For sensitivity analysis of Belowground Grazing algorithm
        >> CatastrophicPlantMortality	// Catastrophic Disturbance: Percent of plant removal during Catastrophic Disturbance
        >> CatastrophicDistWeek			// Catastrophic Disturbance: Week of the disturbance
        >> SeedRainType					// Seed Rain: Off/On/Type
        >> SeedInput						// Seed Rain: Number of seeds to input per SeedRain event
        >> weekly						// Output: Weekly output rather than yearly
        >> ind_out						// Output: Individual-level output
        >> PFT_out						// Output: PFT-level output
        >> srv_out						// Output: End-of-run survival output
        >> trait_out						// Output: Trait-level output
        >> aggregated_out				// Output: Meta-level output
        >> NamePftFile 							// Input: Name of input community (PFT intialization) file
		;

	// set intraspecific competition version, intraspecific trait variation version, and competition modes
	switch (IC_version)
	{
	case 0:
        stabilization = version1;
		break;
	case 1:
        stabilization = version2;
		break;
	case 2:
        stabilization = version3;
		break;
	default:
		break;
	}

	switch (mode)
	{
	case 0:
        mode = communityAssembly;
		break;
	case 1:
        mode = invasionCriterion;
		break;
	case 2:
        if (CatastrophicPlantMortality > 0)
		{
            mode = catastrophicDisturbance;
		}
		else
		{
            mode = communityAssembly;
		}
		break;
	default:
		cerr << "Invalid mode parameterization" << endl;
		exit(1);
	}

    if (mode == invasionCriterion)
	{
        Tmax += Tmax_monoculture;
	}

    if (ITVsd > 0)
	{
        ITV = on;
	}
	else
	{
        ITV = off;
	}

    if (BelGrazPerc > 0)
	{
        BelGrazResidualPerc = exp(-1 * (BelGrazPerc / 0.0651));
	}

	////////////////////
	// Setup PFTs
    NamePftFile = "data/in/" + NamePftFile;

    ////////////////////
    // Design output file names
    const string dir = "data/out/";
    const string fid = outputPrefix;
    string ind;
    string PFT;
    string srv;
    string trait;
    string aggregated;

    string param = 	dir + fid + "_param.csv";
    if (trait_out) {
        trait = 	dir + fid + "_trait.csv";
    }
    if (srv_out) {
        srv = 	dir + fid + "_srv.csv";
    }
    if (PFT_out) {
        PFT = 	dir + fid + "_PFT.csv";
    }
    if (ind_out) {
        ind = 	dir + fid + "_ind.csv";
    }
    if (aggregated_out) {
        aggregated =   dir + fid + "_aggregated.csv";
    }

    output.setupOutput(param, trait, srv, PFT, ind, aggregated);


    ReadPFTDef(NamePftFile);

}

std::string Environment::getSimID()
{

    std::string s =
            std::to_string(SimID) + "_" +
            std::to_string(ComNr) + "_" +
            std::to_string(RunNr);

    return s;
}

void Environment::ReadPFTDef(const string& file)
{
    //Open InitFile
    ifstream InitFile(file.c_str());

    string line;
    getline(InitFile, line); // skip header line
    while (getline(InitFile, line))
    {
        Traits* trait = new Traits(line);
        pftTraitTemplates.insert(std::pair<std::string, Traits*>(trait->PFT_ID, trait));
        pftInsertionOrder.push_back(trait->PFT_ID);
    }
}

/**
 * Retrieve a deep-copy of that PFT's basic trait set
 */
unique_ptr<Traits> Environment::createTraitSetFromPftType(string type)
{
    const auto pos = pftTraitTemplates.find(type);

    return (make_unique<Traits>(*pos->second));
}



