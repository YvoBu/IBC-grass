
#include <iostream>
#include <string>
#include <sstream>

#include "itv_mode.h"
#include "Traits.h"
#include "Environment.h"
#include "RandomGenerator.h"
#include "Plant.h"
#include "Output.h"
#include "IBC-grass.h"

using namespace std;

const int Environment::WeeksPerYear = 30;
extern RandomGenerator rng;
extern long PFTCount;
extern bool pft_ratio_off;

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
        >> mycfbmin
        >> mycfbmax
		;
    //
    //  Calculate range and offset from min and max value.
    mycFbRange  = mycfbmax-mycfbmin;
    mycFbOffset = mycfbmin;

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
        pthread_spin_lock(&cout_lock);
        cerr << "Invalid mode parameterization" << endl;
        pthread_spin_unlock(&cout_lock);
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

    output.setupOutput(param, trait, srv, PFT, ind);


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
    std::ifstream InitFile(file.c_str());
    //
    //  We have three different mycStats and maybe no mycstat at all.
    //  So we define 4 vectors to put the PFTs into it.
    std::vector<Traits> OM;
    std::vector<Traits> FM;
    std::vector<Traits> NM;
    std::vector<Traits> noneM;
    std::vector<Traits> all;

    string line;
    getline(InitFile, line); // skip header line
    while (getline(InitFile, line))
    {
        //
        //  Not only parse the line and setup the values from the line
        //  but also setup the feedback for this PFT. This way all plants
        //  of a PFT have the same feedback.
        Traits trait(line, (rng.get01()*mycFbRange)+mycFbOffset);
        //
        //  We store all read-in PFTs into an all map.
        pftAll.insert(std::pair<std::string, Traits>(trait.PFT_ID, trait));
        all.push_back(trait);
        //  Depending on the mycStat of the new PFT we put the PFT
        //  into the specific vector.
        if (trait.mycStat == "OM") {
            OM.push_back(trait);
        } else if (trait.mycStat == "FM") {
            FM.push_back(trait);
        } else if (trait.mycStat == "NM") {
            NM.push_back(trait);
        } else {
            noneM.push_back(trait);
        }
    }
    if (PFTCount != -1) {
        if (pft_ratio_off) {

            for (int i=0; i<PFTCount; ++i) {
                int index = all.size()*rng.get01();

                pftTraitTemplates.insert(std::pair<std::string, Traits>(all[index].PFT_ID, all[index]));
                pftInsertionOrder.push_back(all[index].PFT_ID);
                //
                //  Cannot erase from an already empty vector
                if (!all.empty()) {
                    all.erase(all.begin()+index);
                }
            }

        } else {
            //
            // We sum the sizes of our PFT vectors for later use
            double count   = OM.size()+FM.size()+NM.size()+noneM.size();

            double factor  = PFTCount/count;
            //
            //  Run it on OMs
            int take = OM.size()*factor+0.5;

            if ((OM.size()>0) && (take == 0)) {
                take=1;
            }
            for (int i=0; i<take; ++i) {
                int index = OM.size()*rng.get01();

                pftTraitTemplates.insert(std::pair<std::string, Traits>(OM[index].PFT_ID, OM[index]));
                pftInsertionOrder.push_back(OM[index].PFT_ID);
                //
                //  Cannot erase from an already empty vector
                if (!OM.empty()) {
                    OM.erase(OM.begin()+index);
                }
            }
            //
            //  Run it on NMs
            take = NM.size()*factor+0.5;

            if ((NM.size()>0) && (take == 0)) {
                take=1;
            }
            for (int i=0; i<take; ++i) {
                int index = NM.size()*rng.get01();

                pftTraitTemplates.insert(std::pair<std::string, Traits>(NM[index].PFT_ID, NM[index]));
                pftInsertionOrder.push_back(NM[index].PFT_ID);
                //
                //  Cannot erase from an already empty vector
                if (!NM.empty()) {
                    NM.erase(NM.begin()+index);
                }
            }
            //
            //  Run it on FMs
            take = FM.size()*factor+0.5;

            if ((FM.size()>0) && (take == 0)) {
                take=1;
            }
            for (int i=0; i<take; ++i) {
                int index = FM.size()*rng.get01();

                pftTraitTemplates.insert(std::pair<std::string, Traits>(FM[index].PFT_ID, FM[index]));
                pftInsertionOrder.push_back(FM[index].PFT_ID);
                //
                //  Cannot erase from an already empty vector
                if (!FM.empty()) {
                    FM.erase(FM.begin()+index);
                }
            }
            //
            //  Run it on noneMs
            take = noneM.size()*factor+0.5;

            if ((noneM.size()>0) && (take == 0)) {
                take=1;
            }
            for (int i=0; i<take; ++i) {
                int index = noneM.size()*rng.get01();

                pftTraitTemplates.insert(std::pair<std::string, Traits>(noneM[index].PFT_ID, noneM[index]));
                pftInsertionOrder.push_back(noneM[index].PFT_ID);
                //
                //  Cannot erase from an already empty vector
                if (!noneM.empty()) {
                    noneM.erase(noneM.begin()+index);
                }
            }
        }
    } else {
        //
        //  Transfer the PFTs into the template map.
        for (std::vector<Traits>::iterator i=OM.begin(); i != OM.end(); ++i) {
            pftTraitTemplates.insert(std::pair<std::string, Traits>(i->PFT_ID, *i));
            pftInsertionOrder.push_back(i->PFT_ID);
        }
        for (std::vector<Traits>::iterator i=NM.begin(); i != NM.end(); ++i) {
            pftTraitTemplates.insert(std::pair<std::string, Traits>(i->PFT_ID, *i));
            pftInsertionOrder.push_back(i->PFT_ID);
        }
        for (std::vector<Traits>::iterator i=FM.begin(); i != FM.end(); ++i) {
            pftTraitTemplates.insert(std::pair<std::string, Traits>(i->PFT_ID, *i));
            pftInsertionOrder.push_back(i->PFT_ID);
        }
        for (std::vector<Traits>::iterator i=noneM.begin(); i != noneM.end(); ++i) {
            pftTraitTemplates.insert(std::pair<std::string, Traits>(i->PFT_ID, *i));
            pftInsertionOrder.push_back(i->PFT_ID);
        }
    }
}


