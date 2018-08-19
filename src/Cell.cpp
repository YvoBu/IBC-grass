
#include <iostream>
#include <algorithm>
#include <cassert>

#include "itv_mode.h"
#include "Cell.h"
#include "RandomGenerator.h"
#include "Output.h"
#include "IBC-grass.h"

using namespace std;

extern RandomGenerator rng;

//-----------------------------------------------------------------------------

Cell::Cell(const unsigned int xx, const unsigned int yy) :
        x(xx), y(yy),
        AResConc(0), BResConc(0),
        aComp_weekly(0), bComp_weekly(0),
        occupied(false)
{
//    AResConc = Parameters::params.meanARes;
//    BResConc = Parameters::params.meanBRes;
}

//-----------------------------------------------------------------------------

Cell::~Cell()
{
    AbovePlantList.clear();
    BelowPlantList.clear();

    SeedBankList.clear();
    SeedlingList.clear();

    PftNIndA.clear();
    PftNIndB.clear();
}

//-----------------------------------------------------------------------------

void Cell::weeklyReset()
{
    AbovePlantList.clear();
    BelowPlantList.clear();

    PftNIndA.clear();
    PftNIndB.clear();

    SeedlingList.clear();
}

//-----------------------------------------------------------------------------

void Cell::SetResource(double Ares, double Bres)
{
   AResConc = Ares;
   BResConc = Bres;
}

//-----------------------------------------------------------------------------

double Cell::Germinate(double weeks)
{
    double sum_SeedMass = 0;

    for (std::vector<Seed>::iterator it = SeedBankList.begin(); it != SeedBankList.end(); )
    {
        //
        //  Division by 8 weeks of establishment.
        if (rng.get01() < (it->pSeedEstab/weeks))
        {
            sum_SeedMass += it->mass;
            SeedlingList.push_back(*it); // This seed germinates, add it to seedlings
            it = SeedBankList.erase(it); // Remove its iterator from the SeedBankList, which now holds only ungerminated seeds
        }
        else
        {
            ++it;
        }
    }

    return sum_SeedMass;
}

void Cell::Germinate(unsigned weeks)
{
    for (std::vector<Seed>::iterator it = SeedBankList.begin(); it != SeedBankList.end(); )
    {
        if (rng.get01() < (it->pSeedEstab)/weeks)
        {
            SeedlingList.push_back(*it); // This seed germinates, add it to seedlings
            it = SeedBankList.erase(it); // Remove its iterator from the SeedBankList, which now holds only ungerminated seeds
        }
        else
        {
            ++it;
        }
    }
}

//-----------------------------------------------------------------------------

void Cell::SeedMortalityWinter(double aMortality) {

    for (std::vector<Seed>::iterator seed=SeedBankList.begin(); seed != SeedBankList.end();)
    {
        if (rng.get01() < aMortality)
        {
#ifdef FAST_ERASE_ON_VECTORS
            *seed = SeedBankList.back();
            SeedBankList.pop_back();
#else
            seed = SeedBankList.erase(seed);
#endif
        }
        else
        {
            ++seed->age;
            ++seed;
        }
    }
}


void Cell::SeedMortalityAge() {
    for (std::vector<Seed>::iterator seed= SeedBankList.begin(); seed != SeedBankList.end();)
    {
        if (seed->OldAge()) {
#ifdef FAST_ERASE_ON_VECTORS
            *seed = SeedBankList.back();
            SeedBankList.pop_back();
#else
            seed = SeedBankList.erase(seed);
#endif
        } else {
            ++seed;
        }
    }
}


void CellAsymPartSymV1::AboveComp() {
    if (AbovePlantList.empty())
        return;

    double comp_tot = 0;
    double comp_c = 0;

    //1. sum of resource requirement
    for (std::vector<Plant*>::iterator pi = AbovePlantList.begin(); pi != AbovePlantList.end(); ++pi)
    {
        comp_tot += (*pi)->comp_coef(1, 2);
    }

    //2. distribute resources
    for (std::vector<Plant*>::iterator pi = AbovePlantList.begin(); pi != AbovePlantList.end(); ++pi)
    {
        comp_c             = (*pi)->comp_coef(1, 2);
        (*pi)->Auptake    += AResConc * comp_c / comp_tot;
        (*pi)->maxAuptake += AResConc;
    }

    aComp_weekly = comp_tot;
}

void CellAsymPartSymV1::BelowComp() {
    if (BelowPlantList.empty())
    {
        return;
    }

    double comp_tot = 0;
    double comp_c = 0;

    //1. sum of resource requirement
    for (std::vector<Plant*>::iterator pi = BelowPlantList.begin(); pi != BelowPlantList.end(); ++pi)
    {
        comp_tot += (*pi)->comp_coef(2, 1);
    }

    //2. distribute resources
    for (std::vector<Plant*>::iterator pi = BelowPlantList.begin(); pi != BelowPlantList.end(); ++pi)
    {
        comp_c = (*pi)->comp_coef(2, 1);
        (*pi)->Buptake    += BResConc * comp_c / comp_tot;
        (*pi)->maxBuptake += BResConc;
    }

    bComp_weekly = comp_tot;
}

void CellAsymPartSymV2::AboveComp() {
    if (AbovePlantList.empty())
        return;

    double comp_tot = 0;
    double comp_c = 0;

    //1. sum of resource requirement
    for (std::vector<Plant*>::iterator pi = AbovePlantList.begin(); pi != AbovePlantList.end(); ++pi)
    {
        comp_tot += (*pi)->comp_coef(1, 2) * prop_res_above((*pi)->pft());
    }

    //2. distribute resources
    for (std::vector<Plant*>::iterator pi = AbovePlantList.begin(); pi != AbovePlantList.end(); ++pi)
    {
        comp_c             = (*pi)->comp_coef(1, 2) * prop_res_above((*pi)->pft());
        (*pi)->Auptake    += (AResConc * comp_c / comp_tot);
        (*pi)->maxAuptake += AResConc;
    }

    aComp_weekly = comp_tot;
}

void CellAsymPartSymV2::BelowComp() {
    if (BelowPlantList.empty())
        return;

    double comp_tot = 0;
    double comp_c = 0;

    //1. sum of resource requirement
    for (std::vector<Plant*>::iterator pi = BelowPlantList.begin(); pi != BelowPlantList.end(); ++pi)
    {
        comp_tot += (*pi)->comp_coef(2, 1) * prop_res_below((*pi)->pft());
    }

    //2. distribute resources
    for (std::vector<Plant*>::iterator pi = BelowPlantList.begin(); pi != BelowPlantList.end(); ++pi)
    {
        comp_c = (*pi)->comp_coef(2, 1) * prop_res_below((*pi)->pft());
        (*pi)->Buptake    += BResConc * comp_c / comp_tot;
        (*pi)->maxBuptake += BResConc;
    }

    bComp_weekly = comp_tot;
}

void CellAsymPartAsymV2::BelowComp() {
    if (BelowPlantList.empty())
        return;

    double comp_tot = 0;
    double comp_c = 0;

    //1. sum of resource requirement
    for (std::vector<Plant*>::iterator pi = BelowPlantList.begin(); pi != BelowPlantList.end(); ++pi)
    {
        comp_tot += (*pi)->comp_coef(2, 2) * prop_res_below((*pi)->pft());
    }

    //2. distribute resources
    for (std::vector<Plant*>::iterator pi = BelowPlantList.begin(); pi != BelowPlantList.end(); ++pi)
    {
        comp_c = (*pi)->comp_coef(2, 2) * prop_res_below((*pi)->pft());
        (*pi)->Buptake    += BResConc * comp_c / comp_tot;
        (*pi)->maxBuptake += BResConc;
    }

    bComp_weekly = comp_tot;
}

double CellAsymPartSymV2::prop_res_above(const string &type) {
    map<string, int>::const_iterator noa = PftNIndA.find(type);
    if (noa != PftNIndA.end())
    {
        return 1.0 / std::sqrt(noa->second);
    }
    return -1.0;
}

double CellAsymPartSymV2::prop_res_below(const string &type) {
    map<string, int>::const_iterator nob = PftNIndB.find(type);
    if (nob != PftNIndB.end())
    {
        return 1.0 / std::sqrt(nob->second);
    }
    return -1;
}

double CellAsymPartSymV3::prop_res_above(const string &type) {
    return PftNIndA.size() / (1.0 + PftNIndA.size());
}

double CellAsymPartSymV3::prop_res_below(const string &type) {
    return PftNIndB.size() / (1.0 + PftNIndB.size());
}
