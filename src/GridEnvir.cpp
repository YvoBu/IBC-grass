#include <memory.h>
#include <iostream>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "timing.h"
#include "itv_mode.h"
#include "Traits.h"
#include "Environment.h"
#include "CThread.h"
#include "Plant.h"
#include "Output.h"
#include "mycorrhiza.h"
#include "Grid.h"
#include "GridEnvir.h"
#include "IBC-grass.h"
using namespace std;
pthread_mutex_t GridEnvir::gammalock;
pthread_mutex_t GridEnvir::aggregated_lock;
//
//  This is the nu,ber if seeds to be put into the grid per PFT.
//  It can be overridden for all runs from the command line.
extern long no_init_seeds;
extern bool output_images;
extern bool timing;
//------------------------------------------------------------------------------

GridEnvir::GridEnvir() {
    OccupantImage = 0;
    CompetitionImage = 0;
}

GridEnvir::~GridEnvir() {
    if (OccupantImage != 0) {
        delete [] OccupantImage;
    }
    if (CompetitionImage != 0) {
        delete [] CompetitionImage;
    }
}

//------------------------------------------------------------------------------
/**
 * Initiate new Run: Randomly set initial individuals.
 */
void GridEnvir::InitRun()
{
    CellsInit();
    InitInds();
    if (output_images) {
        OccupantImage    = new uint32_t[GridSize*GridSize];
        CompetitionImage = new uint32_t[GridSize*GridSize];
    }
}

//-----------------------------------------------------------------------------

void GridEnvir::InitInds()
{
    const double estab = 1.0;

    if (mode == communityAssembly || mode == catastrophicDisturbance)
    {
        // PFT Traits are read in GetSim()
        for (auto const& it : pftTraitTemplates)
        {
            InitSeeds(it.first, no_init_seeds, estab);
            PftSurvTime[it.first] = 0;
        }
    }
    else if (mode == invasionCriterion)
    {
        assert(pftTraitTemplates.size() == 2);

        string resident = pftInsertionOrder[1];
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
        pthread_spin_lock(&cout_lock);
        std::cout << "y " << year;
        pthread_spin_unlock(&cout_lock);
        if (timing) {
            gTimer.Start();
        }

        OneYear();

        pthread_spin_lock(&cout_lock);
        if (timing) {
            std::cout << " needs " << gTimer.End()/1000.0 << " miliseconds to compute";
        }
        std::cout << std::endl;
        pthread_spin_unlock(&cout_lock);


        if (mode == invasionCriterion && year == Tmax_monoculture)
        {
            const int no_init_seeds = 100;
            const double estab = 1.0;

            string invader = pftInsertionOrder[0];
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

    if (year == 1) {
        //
        //  Clear the seedbank in the first year because they are
        //  all have estab=1.0 set.
        Cell* cell;
        for (int i=0; i < getGridArea(); ++i) {
            cell = CellList[i];

            cell->SeedBankList.clear();
        }
    }
#if 0
    //
    //  Create the filename from the SimID only.
    //  Collecting all runs of an experiment in a single file.
    //  But use one file per experiment.
    std::ostringstream  filename;

    filename << "data/out/" << outputPrefix << "_" << SimID << "_masses.csv";
    //
    //  Lockout other threads from accessing the file.
    //pthread_mutex_lock(&GridEnvir::aggregated_lock);
    //
    //  Open file for append.
    std::ofstream aggregatedfile(filename.str(), std::ios_base::app);
    //
    //  Check if file could be opened.
    if (aggregatedfile.good()) {
        //
        //  Check if file is empty.
        if (aggregatedfile.tellp()==0) {
            aggregatedfile << "PlantId, ShootMass, RootMass, dms, dmr, disc, maximum\n";
        }
        for(std::map<int, std::vector< tMassData > >::iterator ai = Plant::allmass.begin(); ai != Plant::allmass.end(); ++ai) {
            for (std::vector<tMassData>::iterator mi = ai->second.begin(); mi != ai->second.end(); ++mi) {
                aggregatedfile << ai->first << ", " << mi->week << "," << mi->shoot << ", " << mi->root << ", " << mi->dms << ", " << mi->dmr << ", " << mi->disc << ", " << mi->maxm  << "," << mi->stress << std::endl;
            }
        }
    }
    Plant::allmass.clear();
#endif
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
//        Disturb();  		// Grazing and disturbances
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
        //
        //  Collect some stats information into the PFT_Stat map.
        //  We are doing it here only once for the different values to create.
        buildPFT_map(PlantList);

        TotalShootmass.push_back(TotalAboveMass);
        TotalRootmass.push_back(TotalBelowMass);
        TotalNonClonalPlants.push_back(GetNPlants());
        TotalClonalPlants.push_back(GetNclonalPlants());
        TotalAboveComp.push_back(GetTotalAboveComp());
        TotalBelowComp.push_back(GetTotalBelowComp());

        print_srv_and_PFT(PlantList);
        //
        //  Check if we should output aggregated data.
        //  No further check is done beyond this point.
        if (aggregated_out == 1)
        {
            print_aggregated(PlantList);
        }

        if (ind_out == 1)
        {
            print_ind(PlantList);
        }
        OutputGamma();
    }
    if (output_images) {
        OutputCompetitionImage();
        OutputOccupantImage();
    }
    myc.UpdatePool();
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
    int FreeCells = 0;

    for (int i = 0; i < getGridArea(); ++i)
    {
        Cell* cell = CellList[i];

        if (cell->ReadyForSeeding()) {
            FreeCells++;
        }
    }




//    std::cerr << "Plants: " << NPlants << " Clonal: " << NClPlants << " Seeds: " << NSeeds << " Free Cells: " << FreeCells << std::endl;
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
    for (auto const& it : pftTraitTemplates)
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
    ss << pftTraitTemplates.size()				<< ", ";
    ss << stabilization 				<< ", ";
    ss << ITVsd 						<< ", ";
    ss << Tmax 						<< ", ";

    if (mode == invasionCriterion)
    {
        std::string invader 	= pftInsertionOrder[0];
        std::string resident 	= pftInsertionOrder[1];

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

void GridEnvir::print_srv_and_PFT(const std::vector< Plant* > & PlantList)
{
    // If any PFT went extinct, record it in "srv" stream
    if (srv_out != 0)
    {
        for (auto it : PFT_Stat)
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
        for (auto it : PFT_Stat)
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
}

void GridEnvir::buildPFT_map(const std::vector< Plant* > & PlantList)
{
    PlantCount     = 0;
    TotalAboveMass = 0.0;
    TotalBelowMass = 0.0;
    PFT_Stat.clear();
    // Aggregate individuals
    for (auto const& p : PlantList)
    {
        if (p->isDead) {
            continue;
        }
        PlantCount++;
        TotalAboveMass += p->mShoot;
        TotalAboveMass += p->mRepro;
        TotalBelowMass += p->mRoot;

        PFT_Stat[p->pft()].Pop++;
        PFT_Stat[p->pft()].Rootmass  += p->mRoot;
        PFT_Stat[p->pft()].Shootmass += p->mShoot;
        PFT_Stat[p->pft()].Repro     += p->mRepro;
    }
}

void GridEnvir::print_trait()
{

    for (auto const& it : pftTraitTemplates)
    {
        std::ostringstream ss;

        ss << getSimID()	<< ", ";
        ss << it.first 						<< ", ";
        ss << it.second.LMR 				<< ", ";
        ss << it.second.m0 				<< ", ";
        ss << it.second.maxMass 			<< ", ";
        ss << it.second.seedMass 			<< ", ";
        ss << it.second.dispersalDist 		<< ", ";
        ss << it.second.SLA 				<< ", ";
        ss << it.second.palat 				<< ", ";
        ss << it.second.Gmax 				<< ", ";
        ss << it.second.memory 			<< ", ";
        ss << it.second.clonal 			<< ", ";
        ss << it.second.meanSpacerlength 	<< ", ";
        ss << it.second.sdSpacerlength 	 << ", "  ;
        ss << it.second.mycStat 	 << ", "  ;
        ss << it.second.mycZOI 	 << ", "  ;
        ss << it.second.mycCOMP 	 << ", "  ;
        ss << it.second.mycC 	  ;



        output.print_row(ss, output.trait_stream);
    }

}


void GridEnvir::print_ind(const std::vector< Plant* > & PlantList)
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
        ss << p->LMR 				<< ", ";
        ss << p->m0 				<< ", ";
        ss << p->maxMass 			<< ", ";
        ss << p->seedMass 			<< ", ";
        ss << p->dispersalDist 		<< ", ";
        ss << p->SLA 				<< ", ";
        ss << p->palat 				<< ", ";
        ss << p->Gmax 				<< ", ";
        ss << p->memory 			<< ", ";
        ss << p->clonal 			<< ", ";
        ss << p->meanSpacerlength 	<< ", ";
        ss << p->sdSpacerlength 	<< ", ";
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
void GridEnvir::print_aggregated(const std::vector< Plant* > & PlantList)
{
    std::map<std::string, double> meanTraits = output.calculateMeanTraits(PlantList);

    std::ostringstream ss;

    ss << getSimID() 											<< ", ";
    ss << Environment::year															<< ", ";
    ss << Environment::week 														<< ", ";
    if (BlwgrdGrazingPressure.empty()) {
        ss << "0.0, ";
    } else {
        ss << BlwgrdGrazingPressure.back()                                              << ", ";
    }
    if (ContemporaneousRootmassHistory.empty()) {
        ss << "0.0, ";
    } else {
        ss << ContemporaneousRootmassHistory.back()                                 	<< ", ";
    }
    double shannon_div = output.calculateShannon(PFT_Stat, PlantCount);
    ss <<  shannon_div	<< ", ";
    //
    //  The richness is the number of entries in the PFT_Stat map.
    ss << PFT_Stat.size() << ",";

    double brayCurtis = calculateBrayCurtis(PFT_Stat, CatastrophicDistYear - 1, year);
    if (!Environment::AreSame(brayCurtis, -1))
    {
        ss << brayCurtis 															<< ", ";
    }
    else
    {
        ss << "NA"																	<< ", ";
    }
    if (TotalAboveComp.empty()) {
        ss << "0.0, ";
    } else {
        ss << TotalAboveComp.back()                                                     << ", ";
    }
    if (TotalBelowComp.empty()) {
        ss << "0.0, ";
    } else {
        ss << TotalBelowComp.back()                                                     << ", ";
    }
    ss << TotalAboveMass << ",";
    ss << TotalBelowMass << ", ";
    if (TotalNonClonalPlants.empty()) {
        ss << "0.0, ";
    } else {
        ss << TotalNonClonalPlants.back()                                                     << ", ";
    }
    if (TotalClonalPlants.empty()) {
        ss << "0.0, ";
    } else {
        ss << TotalClonalPlants.back()                                                     << ", ";
    }

    ss << meanTraits["LMR"] 														<< ", ";
    ss << meanTraits["MaxMass"] 													<< ", ";
    ss << meanTraits["Gmax"] 														<< ", ";
    ss << meanTraits["SLA"] 													    << ", ";
    ss << GetMycStatCount("OM")													    << ", ";
    ss << GetMycStatCount("FM")  												    << ", ";
    ss << GetMycStatCount("NM") 													<< ", ";
    ss << (shannon_div / (-log(1.0/PlantCount)));
    //
    //  Create the filename from the SimID only.
    //  Collecting all runs of an experiment in a single file.
    //  But use one file per experiment.
    std::ostringstream  filename;

    filename << "data/out/" << outputPrefix << "_" << SimID << "_aggregated.csv";
    //
    //  Lockout other threads from accessing the file.
    pthread_mutex_lock(&GridEnvir::aggregated_lock);
    //
    //  Open file for append.
    std::ofstream aggregatedfile(filename.str(), std::ios_base::app);
    //
    //  Check if file could be opened.
    if (aggregatedfile.good()) {
        //
        //  Check if file is empty.
        if (aggregatedfile.tellp()==0) {
            std::vector<std::string>::const_iterator hi;
            //
            //  The header for the aggregated file is already defined in the Output-Class.
            for(hi = Output::aggregated_header.begin(); hi != Output::aggregated_header.end(); ++hi) {
                //
                //  The first header-item has no comma prepended.
                if (hi != Output::aggregated_header.begin()) {
                    aggregatedfile << ",";
                }
                //
                //  Output the header-item
                aggregatedfile << *hi;
            }
            aggregatedfile << std::endl;
        }
        //
        //  Output the real data.
        aggregatedfile << ss.str();
        //
        // Write the counts for the pfts.
        aggregatedfile << std::endl;
    } else {
        pthread_spin_lock(&cout_lock);
        std::cerr << "Could not open aggregatedfile " << filename.str() << std::endl;
        pthread_spin_unlock(&cout_lock);
    }
    //
    // Allow access from other runs.
    pthread_mutex_unlock(&GridEnvir::aggregated_lock);
}

void GridEnvir::OutputGamma() {
    //
    //  Lockout all other threads.
    pthread_mutex_lock(&GridEnvir::gammalock);
    std::map<std::string, long> pft;
    std::map<std::string, long>::iterator pfti;
    std::vector< Plant* >::iterator pi;
    //
    //  Create counters from all PFTs
    std::map< std::string, Traits >::iterator ti;

    for (ti = pftAll.begin(); ti != pftAll.end(); ++ti) {
        pft[ti->first] = 0;
    }
    //
    //  Count PFTs
    for (pi=PlantList.begin(); pi != PlantList.end(); ++pi) {
        if (!(*pi)->isDead) {
            pft[(*pi)->PFT_ID]++;
        }
    }
    //
    //  Generate output file
    std::ostringstream oss;
    //
    //  Create filename from different variables inside the simulation
    //  SimID is an int. So we need to format it.
    oss << "data/out/" << outputPrefix << "_" << SimID << "_gamma.csv";
    //
    //  Open file for append.
    std::ofstream gammafile(oss.str(), std::ios_base::app);
    //
    //  Check if file could be opened.
    if (gammafile.good()) {
        //
        //  Check if file is empty.
        if (gammafile.tellp()==0) {
            //
            // In an empty file we create the header first.
            gammafile << "RunNr,Year";
            //
            //  Put the PFT_IDs as field header.
            for (pfti = pft.begin(); pfti != pft.end(); ++pfti) {
                gammafile << "," << pfti->first;
            }
            gammafile << ",OMcount, FMcount, NMcount";
            gammafile << std::endl;
        }
        //
        //  Output the real data.
        gammafile << RunNr << "," << year;
        //
        // Write the counts for the pfts.
        for (pfti = pft.begin(); pfti != pft.end(); ++pfti) {
            gammafile << "," << pfti->second;
        }

        long omc = 0;
        long fmc = 0;
        long nmc = 0;
        std::map<std::string, Traits>::iterator ti;

        for (pfti = pft.begin(); pfti != pft.end(); ++pfti) {
            if (pfti->second != 0) {
                ti = pftTraitTemplates.find(pfti->first);
                if (ti != pftTraitTemplates.end()) {
                    if (ti->second.mycStat == "OM") {
                        omc++;
                    } else if (ti->second.mycStat == "FM") {
                        fmc++;
                    } else if (ti->second.mycStat == "NM") {
                        nmc++;
                    }
                }
            }
        }
        gammafile << "," << omc << "," << fmc << "," << nmc;

        gammafile << std::endl;
    } else {
        pthread_spin_lock(&cout_lock);
        std::cerr << "Could not open gammafile " << oss.str() << std::endl;
        pthread_spin_unlock(&cout_lock);
    }
    //
    // Let other threads do their output.
    pthread_mutex_unlock(&GridEnvir::gammalock);
}


/*
 * benchmarkYear is generally the year to prior to disturbance
 * BC_window is the length of the time period (years) in which PFT populations are averaged to arrive at a stable mean for comparison
 */
double GridEnvir::calculateBrayCurtis(const std::map<std::string, PFT_struct> & _PFT_map, int benchmarkYear, int theYear)
{
    static const int BC_window = 10;

    // Preparing the "average population counts" in the years preceding the catastrophic disturbance
    if ((theYear > (benchmarkYear - BC_window)) && (theYear <= benchmarkYear))
    {
        // Add this year's population to the PFT's abundance sum over the window
        for (auto& pft : _PFT_map)
        {
            BC_predisturbance_Pop[pft.first] += pft.second.Pop;
        }

        // If it's the last year before disturbance, divide the population count by the window
        if (theYear == benchmarkYear)
        {
            for (auto& pft_total : BC_predisturbance_Pop)
            {
                pft_total.second = pft_total.second / BC_window;
            }
        }
    }

    if (theYear <= benchmarkYear)
    {
        return -1;
    }

    std::vector<int> popDistance;
    for (auto pft : _PFT_map)
    {
        popDistance.push_back( abs( BC_predisturbance_Pop[pft.first] - pft.second.Pop ) );
    }

    int BC_distance_sum = std::accumulate(popDistance.begin(), popDistance.end(), 0);

    int present_totalAbundance = std::accumulate(_PFT_map.begin(), _PFT_map.end(), 0,
                                [] (int s, const std::map<string, PFT_struct>::value_type& p)
                                {
                                    return s + p.second.Pop;
                                });

    int past_totalAbundance = std::accumulate(BC_predisturbance_Pop.begin(), BC_predisturbance_Pop.end(), 0,
                                [] (int s, const std::map<string, int>::value_type& p)
                                {
                                    return s + p.second;
                                });

    int BC_abundance_sum = present_totalAbundance + past_totalAbundance;

    return BC_distance_sum / (double) BC_abundance_sum;
}

void GridEnvir::OutputOccupantImage() {
    memset(OccupantImage, 0, GridSize*GridSize*sizeof(OccupantImage[0]));

#if 1
    for (int x = 0; x < GridSize; x++) {
        for (int y = 0; y < GridSize; y++) {
            if (CellList[x + (y*GridSize)]->occupied) {
                OccupantImage[x + (y*GridSize)] = 0x000000ffu;
            }
        }
    }
#endif

    std::ostringstream ofile;

    ofile << "./data/out/occupant" << std::setfill('0') << std::setw(4) << year <<std::setw(2) << week << ".raw";
    std::ofstream img(ofile.str(), std::ios::binary | std::ios::out | std::ios::trunc);

    img.write((const char*)OccupantImage, GridSize*GridSize*sizeof(OccupantImage[0]));

}

void GridEnvir::OutputCompetitionImage() {
    memset(CompetitionImage, 0, GridSize*GridSize*sizeof(CompetitionImage[0]));

    for (int x = 0; x < GridSize; x++) {
        for (int y = 0; y < GridSize; y++) {
            CompetitionImage[x + (y*GridSize)] = (25u << 8) * CellList[x + (y*GridSize)]->AbovePlantList.size();
        }
    }
    std::ostringstream ofile;

    ofile << "./data/out/competition" << std::setfill('0') << std::setw(4) << year <<std::setw(2) << week << ".raw";
    std::ofstream img(ofile.str(), std::ios::binary | std::ios::out | std::ios::trunc);

    img.write((const char*)CompetitionImage, GridSize*GridSize*sizeof(CompetitionImage[0]));

}
