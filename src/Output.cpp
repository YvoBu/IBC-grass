#include <iostream>
#include <sstream>
#include <iterator>
#include <cassert>
#include <math.h>

#include "itv_mode.h"
#include "Plant.h"
#include "Output.h"
#include "Environment.h"

using namespace std;


const vector<string> Output::param_header
    ({
         "SimID", "ComNr", "RunNr", "nPFTs",
         "IC_vers", "ITVsd", "Tmax",
         "Invader", "Resident",
         "ARes", "BRes",
         "GrazProb", "PropRemove",
         "BelGrazProb", "BelGrazPerc",
         "BelGrazAlpha", "BelGrazHistorySize",
         "CatastrophicMortality", "CatastrophicDistWeek",
         "SeedRainType", "SeedInput"
    });

const vector<string> Output::trait_header
    ({
         "SimID", "PFT",
         "LMR",
         "m0", "MaxMass", "SeedMass", "Dist",
         "SLA", "palat",
         "Gmax", "memory",
         "clonal", "meanSpacerlength", "sdSpacerlength"
    });

const vector<string> Output::srv_header
    ({
         "SimID", "PFT", "Extinction_Year", "Final_Pop", "Final_Shootmass", "Final_Rootmass"
    });

const vector<string> Output::PFT_header
    ({
         "SimID", "PFT", "Year", "Week", "Pop", "Shootmass", "Rootmass", "Repro"
    });

const vector<string> Output::aggregated_header
    ({
         "SimID", "Year", "Week",
         "FeedingPressure", "ContemporaneousRootmass",
         "Shannon", "Richness", "BrayCurtisDissimilarity",
         "TotalAboveComp", "TotalBelowComp",
         "TotalShootmass", "TotalRootmass",
         "TotalNonClonalPlants", "TotalClonalPlants",
         "wm_LMR", "wm_MaxMass", "wm_Gmax", "wm_SLA", "OMcount", "FMcount", "NMcount"
    });

const vector<string> Output::ind_header
    ({
         "SimID", "plantID", "PFT", "Year", "Week",
         "i_X", "i_Y",
         "i_LMR",
         "i_m0", "i_MaxMass", "i_SeedMass", "i_Dist",
         "i_SLA", "i_palat",
         "i_Gmax", "i_memory",
         "i_clonal", "i_meanSpacerlength", "i_sdSpacerlength",
         "i_genetID", "i_Age",
         "i_mShoot", "i_mRoot", "i_rShoot", "i_rRoot",
         "i_mRepro", "i_lifetimeFecundity",
         "i_stress"
    });


Output::Output() :
        param_fn("data/out/param.txt"),
        trait_fn("data/out/trait.txt"),
        srv_fn("data/out/srv.txt"),
        PFT_fn("data/out/PFT.txt"),
        ind_fn("data/out/ind.txt")
{
#if 0
    BlwgrdGrazingPressure = { 0 };
    ContemporaneousRootmassHistory = { 0 };
    TotalShootmass = { 0 };
    TotalRootmass = { 0 };
    TotalAboveComp = { 0 };
    TotalBelowComp = { 0 };
    TotalNonClonalPlants = { 0 };
    TotalClonalPlants = { 0 };
#endif
    pthread_mutex_init(&outputlock, 0);
    pthread_mutex_init(&openlock, 0);
}

Output::~Output()
{
    cleanup();
}

void Output::setupOutput(string _param_fn, string _trait_fn, string _srv_fn,
                         string _PFT_fn, string _ind_fn)
{

    pthread_mutex_lock(&openlock);

    param_fn = _param_fn;
    trait_fn = _trait_fn;
    srv_fn = _srv_fn;
    PFT_fn = _PFT_fn;
    ind_fn = _ind_fn;

    bool mid_batch = is_file_exist(param_fn.c_str());

    if (!param_stream.is_open()) {
        param_stream.open(param_fn.c_str(), ios_base::app);
        assert(param_stream.good());
        if (!mid_batch) print_row(param_header, param_stream);
    }
    if ((!trait_fn.empty()) && (!trait_stream.is_open()))
    {
        trait_stream.open(trait_fn.c_str(), ios_base::app);
        assert(trait_stream.good());
        if (!mid_batch) print_row(trait_header, trait_stream);
    }

    if ((!PFT_fn.empty()) && (!PFT_stream.is_open()))
    {
        PFT_stream.open(PFT_fn.c_str(), ios_base::app);
        assert(PFT_stream.good());
        if (!mid_batch) print_row(PFT_header, PFT_stream);
    }

    if ((!ind_fn.empty()) && (!ind_stream.is_open()))
    {
        ind_stream.open(ind_fn.c_str(), ios_base::app);
        assert(ind_stream.good());
        if (!mid_batch) print_row(ind_header, ind_stream);
    }

    if ((!srv_fn.empty()) && (!srv_stream.is_open()))
    {
        srv_stream.open(srv_fn.c_str(), ios_base::app);
        assert(srv_stream.good());
        if (!mid_batch) print_row(srv_header, srv_stream);
    }
    pthread_mutex_unlock(&openlock);
}

bool Output::is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Output::cleanup()
{
    if (Output::param_stream.is_open()) {
        Output::param_stream.close();
        Output::param_stream.clear();
    }

    if (Output::trait_stream.is_open()) {
        Output::trait_stream.close();
        Output::trait_stream.clear();
    }

    if (Output::srv_stream.is_open()) {
        Output::srv_stream.close();
        Output::srv_stream.clear();
    }

    if (Output::PFT_stream.is_open()) {
        Output::PFT_stream.close();
        Output::PFT_stream.clear();
    }

    if (Output::ind_stream.is_open()) {
        Output::ind_stream.close();
        Output::ind_stream.clear();
    }
}





// Prints a row of data out a string, as a comma separated list with a newline at the end.
void Output::print_row(std::ostringstream & ss, ofstream & stream)
{
    pthread_mutex_lock(&outputlock);
    assert(stream.good());

    stream << ss.str() << endl;

    stream.flush();
    pthread_mutex_unlock(&outputlock);
}

void Output::print_row(vector<string> row, ofstream & stream)
{
    pthread_mutex_lock(&outputlock);
    assert(stream.good());

    std::ostringstream ss;

    std::copy(row.begin(), row.end() - 1, std::ostream_iterator<string>(ss, ", "));

    ss << row.back();

    stream << ss.str() << endl;

    stream.flush();
    pthread_mutex_unlock(&outputlock);
}

double Output::calculateShannon(const std::map<std::string, PFT_struct> & _PFT_map)
{
    int totalPop = std::accumulate(_PFT_map.begin(), _PFT_map.end(), 0,
                        [] (int s, const std::map<string, PFT_struct>::value_type& p)
                        {
                            return s + p.second.Pop;
                        });

    map<string, double> pi_map;

    for (auto pft : _PFT_map)
    {
        if (pft.second.Pop > 0)
        {
            double propPFT = pft.second.Pop / (double) totalPop;
            pi_map[pft.first] = propPFT * log(propPFT);
        }
    }

    double total_Pi_ln_Pi = std::accumulate(pi_map.begin(), pi_map.end(), 0.0,
                                [] (double s, const std::map<string, double>::value_type& p)
                                {
                                    return s + p.second;
                                });

    if (Environment::AreSame(total_Pi_ln_Pi, 0))
    {
        return 0;
    }

    return (-1.0 * total_Pi_ln_Pi);
}

double Output::calculateRichness(const std::map<std::string, PFT_struct> & _PFT_map)
{
    int richness = std::accumulate(_PFT_map.begin(), _PFT_map.end(), 0,
                        [] (int s, const std::map<string, PFT_struct>::value_type& p)
                        {
                            if (p.second.Pop > 0)
                            {
                                return s + 1;
                            }
                            return s;
                        });

    return richness;
}
#if 0
/*
 * benchmarkYear is generally the year to prior to disturbance
 * BC_window is the length of the time period (years) in which PFT populations are averaged to arrive at a stable mean for comparison
 */
double Output::calculateBrayCurtis(const std::map<std::string, PFT_struct> & _PFT_map, int benchmarkYear, int theYear)
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
#endif
std::map<std::string, double> Output::calculateMeanTraits(const std::vector< Plant* > & PlantList)
{
    std::map<std::string, double> weightedMeanTraits;
    int pop = 0;

    for (auto const& p : PlantList)
    {
        if (p->isDead)
        {
            continue;
        }

        weightedMeanTraits["LMR"] += p->LMR;
        weightedMeanTraits["MaxMass"] += p->maxMass;
        weightedMeanTraits["Gmax"] += p->Gmax;
        weightedMeanTraits["SLA"] += p->SLA;

        ++pop;
    }

    for (auto& trait : weightedMeanTraits)
    {
        trait.second = trait.second / pop;
    }

    return weightedMeanTraits;
}
