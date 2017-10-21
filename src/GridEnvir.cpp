
#include <iostream>
#include <cassert>
#include <sstream>
#include "itv_mode.h"
#include "Traits.h"
#include "Environment.h"
#include "CThread.h"
#include "Output.h"
#include "Grid.h"
#include "GridEnvir.h"
#include "IBC-grass.h"
using namespace std;

//------------------------------------------------------------------------------

GridEnvir::GridEnvir() { }

//------------------------------------------------------------------------------
/**
 * Initiate new Run: Randomly set initial individuals.
 */
void GridEnvir::InitRun()
{
    CellsInit();
    InitInds();
}

//-----------------------------------------------------------------------------

void GridEnvir::InitInds()
{
    const int no_init_seeds = 10;
    const double estab = 1.0;

    if (mode == communityAssembly || mode == catastrophicDisturbance)
    {
        // PFT Traits are read in GetSim()
        for (auto const& it : traits.pftTraitTemplates)
        {
            InitSeeds(it.first, no_init_seeds, estab);
            PftSurvTime[it.first] = 0;
        }
    }
    else if (mode == invasionCriterion)
    {
        assert(traits.pftTraitTemplates.size() == 2);

        string resident = traits.pftInsertionOrder[1];
        InitSeeds(resident, no_init_seeds, estab);
        PftSurvTime[resident] = 0;
    }
}

//-----------------------------------------------------------------------------

void GridEnvir::OneRun()
{
    print_param();

    if (trait_out)
    {
        print_trait();
    }

    do {
        std::cout << "y " << year << std::endl;

        OneYear();

        if (mode == invasionCriterion && year == Tmax_monoculture)
        {
            const int no_init_seeds = 100;
            const double estab = 1.0;

            string invader = traits.pftInsertionOrder[0];
            InitSeeds(invader, no_init_seeds, estab);
            PftSurvTime[invader] = 0;
        }

        if (exitConditions())
        {
            break;
        }

    } while (++year <= Tmax);

}

//-----------------------------------------------------------------------------

void GridEnvir::OneYear()
{
    week = 1;

    do {
        //std::cout << "y " << year << " w " << week << std::endl;

        OneWeek();

        if (exitConditions()) break;

    } while (++week <= WeeksPerYear);
}

//-----------------------------------------------------------------------------

void GridEnvir::OneWeek()
{

    ResetWeeklyVariables(); // Clear ZOI data
    SetCellResources();     // Restore/modulate cell resources

    CoverCells();          	// Calculate zone of influences (ZOIs)
    DistributeResource();   // Allot resources based on ZOI

    PlantLoop();            // Growth, dispersal, mortality

    if (year > 1)
    {
        Disturb();  		// Grazing and disturbances
    }

    if (mode == catastrophicDisturbance 						// Catastrophic disturbance is on
            && Environment::year == CatastrophicDistYear 	// It is the disturbance year
            && Environment::week == CatastrophicDistWeek) 	// It is the disturbance week
    {
        RunCatastrophicDisturbance();
    }

    RemovePlants();    		// Remove decomposed plants and remove them from their genets

    if (SeedRainType > 0 && week == 21)
    {
        SeedRain();
    }

    EstablishmentLottery(); // for seeds and ramets

    if (week == 20)
    {
        SeedMortalityAge(); // Remove non-dormant seeds before autumn
    }

    if (week == WeeksPerYear)
    {
        Winter();           	// removal of aboveground biomass and decomposed plants
        SeedMortalityWinter();  // winter seed mortality
    }

    if ((weekly == 1 || week == 20) &&
            !(mode == invasionCriterion &&
                    Environment::year <= Tmax_monoculture)) // Not a monoculture
    {
        output.TotalShootmass.push_back(GetTotalAboveMass());
        output.TotalRootmass.push_back(GetTotalBelowMass());
        output.TotalNonClonalPlants.push_back(GetNPlants());
        output.TotalClonalPlants.push_back(GetNclonalPlants());
        output.TotalAboveComp.push_back(GetTotalAboveComp());
        output.TotalBelowComp.push_back(GetTotalBelowComp());

        print_srv_and_PFT(PlantList);

        if (aggregated_out == 1)
        {
            print_aggregated(PlantList);
        }

        if (ind_out == 1)
        {
            print_ind(PlantList);
        }
    }

}

//-----------------------------------------------------------------------------

bool GridEnvir::exitConditions()
{
    // Exit conditions do not exist with external seed input
    if (SeedInput > 0)
        return false;

    int NPlants = GetNPlants();
    int NClPlants = GetNclonalPlants();
    int NSeeds = GetNSeeds();

    // if no more individuals existing
    if ((NPlants + NClPlants + NSeeds) == 0) {
        return true; //extinction time
    }

    return false;
}

//-----------------------------------------------------------------------------

void GridEnvir::SeedRain()
{

    // For each PFT, we'll drop n seeds
    for (auto const& it : traits.pftTraitTemplates)
    {
        auto pft_name = it.first;
        double n;

        switch (SeedRainType)
        {
            case 1:
                n = SeedInput;
                break;
            default:
                exit(1);
        }

        Grid::InitSeeds(pft_name, n, 1.0);
    }

}


void GridEnvir::print_param()
{
    std::ostringstream ss;

    ss << getSimID()					<< ", ";
    ss << Environment::ComNr 							<< ", ";
    ss << Environment::RunNr 							<< ", ";
    ss << traits.pftTraitTemplates.size()				<< ", ";
    ss << stabilization 				<< ", ";
    ss << ITVsd 						<< ", ";
    ss << Tmax 						<< ", ";

    if (mode == invasionCriterion)
    {
        std::string invader 	= traits.pftInsertionOrder[0];
        std::string resident 	= traits.pftInsertionOrder[1];

        ss << invader 									<< ", ";
        ss << resident 									<< ", ";
    }
    else
    {
        ss << "NA"	 									<< ", ";
        ss << "NA"	 									<< ", ";
    }

    ss << meanARes 					<< ", ";
    ss << meanBRes 					<< ", ";
    ss << AbvGrazProb 				<< ", ";
    ss << AbvPropRemoved 			<< ", ";
    ss << BelGrazProb 				<< ", ";
    ss << BelGrazPerc 				<< ", ";
    ss << BelGrazAlpha				<< ", ";
    ss << BelGrazHistorySize			<< ", ";
    ss << CatastrophicPlantMortality << ", ";
    ss << CatastrophicDistWeek 		<< ", ";
    ss << SeedRainType 				<< ", ";
    ss << SeedInput						   ;

    output.print_row(ss, output.param_stream);
}

void GridEnvir::print_srv_and_PFT(const std::vector< std::shared_ptr<Plant> > & PlantList)
{

    // Create the data structure necessary to aggregate individuals
    auto PFT_map = buildPFT_map(PlantList);

    // If any PFT went extinct, record it in "srv" stream
    if (srv_out != 0)
    {
        for (auto it : PFT_map)
        {
            if ((Environment::PftSurvTime[it.first] == 0 && it.second.Pop == 0) ||
                    (Environment::PftSurvTime[it.first] == 0 && Environment::year == Tmax))
            {
                Environment::PftSurvTime[it.first] = Environment::year;

                std::ostringstream s_ss;

                s_ss << getSimID()	<< ", ";
                s_ss << it.first 						<< ", "; // PFT name
                s_ss << Environment::year				<< ", ";
                s_ss << it.second.Pop 					<< ", ";
                s_ss << it.second.Shootmass 			<< ", ";
                s_ss << it.second.Rootmass 					   ;

                output.print_row(s_ss, output.srv_stream);
            }
        }
    }

    // If one should print PFTs, do so.
    if (PFT_out != 0)
    {
        // print each PFT
        for (auto it : PFT_map)
        {
            if (PFT_out == 1 &&
                    it.second.Pop == 0 &&
                    Environment::PftSurvTime[it.first] != Environment::year)
            {
                continue;
            }

            std::ostringstream p_ss;

            p_ss << getSimID()	<< ", ";
            p_ss << it.first 						<< ", "; // PFT name
            p_ss << Environment::year 				<< ", ";
            p_ss << Environment::week 				<< ", ";
            p_ss << it.second.Pop 					<< ", ";
            p_ss << it.second.Shootmass 			<< ", ";
            p_ss << it.second.Rootmass 				<< ", ";
            p_ss << it.second.Repro 					   ;

            output.print_row(p_ss, output.PFT_stream);
        }
    }

    // delete PFT_map
    PFT_map.clear();
}

map<string, PFT_struct> GridEnvir::buildPFT_map(const std::vector< std::shared_ptr<Plant> > & PlantList)
{
    map<string, PFT_struct> PFT_map;

    for (auto const& it : traits.pftTraitTemplates)
    {
        PFT_map[it.first] = PFT_struct();
    }

    // Aggregate individuals
    for (auto const& p : PlantList)
    {
        if (p->isDead)
            continue;

        PFT_struct* s = &(PFT_map[p->pft()]);

        s->Pop = s->Pop + 1;
        s->Rootmass = s->Rootmass + p->mRoot;
        s->Shootmass = s->Shootmass + p->mShoot;
        s->Repro = s->Repro + p->mRepro;
    }

    return PFT_map;
}

void GridEnvir::print_trait()
{

    for (auto const& it : traits.pftTraitTemplates)
    {
        std::ostringstream ss;

        ss << getSimID()	<< ", ";
        ss << it.first 						<< ", ";
        ss << it.second->LMR 				<< ", ";
        ss << it.second->m0 				<< ", ";
        ss << it.second->maxMass 			<< ", ";
        ss << it.second->seedMass 			<< ", ";
        ss << it.second->dispersalDist 		<< ", ";
        ss << it.second->SLA 				<< ", ";
        ss << it.second->palat 				<< ", ";
        ss << it.second->Gmax 				<< ", ";
        ss << it.second->memory 			<< ", ";
        ss << it.second->clonal 			<< ", ";
        ss << it.second->meanSpacerlength 	<< ", ";
        ss << it.second->sdSpacerlength 	       ;

        output.print_row(ss, output.trait_stream);
    }

}


void GridEnvir::print_ind(const std::vector< std::shared_ptr<Plant> > & PlantList)
{
    for (auto const& p : PlantList)
    {
        if (p->isDead) continue;

        std::ostringstream ss;

        ss << getSimID()	<< ", ";
        ss << p->plantID 					<< ", ";
        ss << p->pft() 						<< ", ";
        ss << Environment::year 			<< ", ";
        ss << Environment::week 			<< ", ";
        ss << p->y 							<< ", ";
        ss << p->x 							<< ", ";
        ss << p->traits->LMR 				<< ", ";
        ss << p->traits->m0 				<< ", ";
        ss << p->traits->maxMass 			<< ", ";
        ss << p->traits->seedMass 			<< ", ";
        ss << p->traits->dispersalDist 		<< ", ";
        ss << p->traits->SLA 				<< ", ";
        ss << p->traits->palat 				<< ", ";
        ss << p->traits->Gmax 				<< ", ";
        ss << p->traits->memory 			<< ", ";
        ss << p->traits->clonal 			<< ", ";
        ss << p->traits->meanSpacerlength 	<< ", ";
        ss << p->traits->sdSpacerlength 	<< ", ";
        ss << p->genet.lock()->genetID		<< ", ";
        ss << p->age 						<< ", ";
        ss << p->mShoot						<< ", ";
        ss << p->mRoot 						<< ", ";
        ss << p->Radius_shoot() 			<< ", ";
        ss << p->Radius_root() 				<< ", ";
        ss << p->mRepro 					<< ", ";
        ss << p->lifetimeFecundity 			<< ", ";
        ss << p->isStressed						   ;

        output.print_row(ss, output.ind_stream);
    }
}
void GridEnvir::print_aggregated(const std::vector< std::shared_ptr<Plant> > & PlantList)
{

    auto PFT_map = buildPFT_map(PlantList);

    std::map<std::string, double> meanTraits = output.calculateMeanTraits(PlantList);

    std::ostringstream ss;

    ss << getSimID() 											<< ", ";
    ss << Environment::year															<< ", ";
    ss << Environment::week 														<< ", ";
    ss << output.BlwgrdGrazingPressure.back()                                              << ", ";
    ss << output.ContemporaneousRootmassHistory.back()                                 	<< ", ";
    ss << output.calculateShannon(PFT_map) 												<< ", ";
    ss << output.calculateRichness(PFT_map)												<< ", ";

    double brayCurtis = output.calculateBrayCurtis(PFT_map, CatastrophicDistYear - 1, year);
    if (!Environment::AreSame(brayCurtis, -1))
    {
        ss << brayCurtis 															<< ", ";
    }
    else
    {
        ss << "NA"																	<< ", ";
    }

    ss << output.TotalAboveComp.back()                                                     << ", ";
    ss << output.TotalBelowComp.back()                                                     << ", ";
    ss << output.TotalShootmass.back()                                                     << ", ";
    ss << output.TotalRootmass.back()                                                      << ", ";
    ss << output.TotalNonClonalPlants.back()                                               << ", ";
    ss << output.TotalClonalPlants.back()                                                  << ", ";
    ss << meanTraits["LMR"] 														<< ", ";
    ss << meanTraits["MaxMass"] 													<< ", ";
    ss << meanTraits["Gmax"] 														<< ", ";
    ss << meanTraits["SLA"] 													           ;

    output.print_row(ss, output.aggregated_stream);

}
