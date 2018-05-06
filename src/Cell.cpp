
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

double Cell::Germinate()
{
    double sum_SeedMass = 0;

    for (std::vector<Seed>::iterator it = SeedBankList.begin(); it != SeedBankList.end(); )
    {
        if (rng.get01() < it->pSeedEstab)
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

#if 0
std::vector<Seed> Cell::Germinate()
{
    double sum_SeedMass = 0;
    std::vector<Seed> retval;

    for (std::vector<Seed>::iterator it = SeedBankList.begin(); it != SeedBankList.end(); )
    {
        if (rng.get01() < it->pEstab)
        {
            sum_SeedMass += it->mass;
            retval.push_back(*it);        // This seed germinates, add it to seedlings
            it = SeedBankList.erase(it);  // Remove its iterator from the SeedBankList, which now holds only ungerminated seeds
        }
        else
        {
            ++it;
        }
    }

    return retval;
}
#endif

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

//-----------------------------------------------------------------------------
#if 0

void Cell::AboveComp()
{
    if (AbovePlantList.empty())
        return;

    if (Parameters::params.AboveCompMode == asymtot)
    {
        weak_ptr<Plant> p_ptr =
                *std::max_element(AbovePlantList.begin(), AbovePlantList.end(),
                        [](const weak_ptr<Plant> & a, const weak_ptr<Plant> & b)
                        {
                            auto _a = a.lock();
                            auto _b = b.lock();

                            return Plant::getShootGeometry(_a) < Plant::getShootGeometry(_b);
                        });

        auto p = p_ptr.lock();

        assert(p);

        p->Auptake += AResConc;

        return;
    }

    int symm;
    if (Parameters::params.AboveCompMode == asympart)
    {
        symm = 2;
    }
    else
    {
        symm = 1;
    }

    double comp_tot = 0;
    double comp_c = 0;

    //1. sum of resource requirement
    for (auto const& plant_ptr : AbovePlantList)
    {
        auto plant = plant_ptr.lock();

        comp_tot += plant->comp_coef(1, symm) * prop_res(plant->pft(), 1, Parameters::params.stabilization);
    }

    //2. distribute resources
    for (auto const& plant_ptr : AbovePlantList)
    {
        auto plant = plant_ptr.lock();
        assert(plant);

        comp_c = plant->comp_coef(1, symm) * prop_res(plant->pft(), 1, Parameters::params.stabilization);
        plant->Auptake += AResConc * comp_c / comp_tot;
    }

    aComp_weekly = comp_tot;
}

//-----------------------------------------------------------------------------

void Cell::BelowComp()
{
    assert(Parameters::params.BelowCompMode != asymtot);

    if (BelowPlantList.empty())
        return;

    int symm;
    if (Parameters::params.BelowCompMode == asympart)
    {
        symm = 2;
    }
    else
    {
        symm = 1;
    }

    double comp_tot = 0;
    double comp_c = 0;

    //1. sum of resource requirement
    for (auto const& plant_ptr : BelowPlantList)
    {
        auto plant = plant_ptr.lock();
        assert(plant);

        comp_tot += plant->comp_coef(2, symm) * prop_res(plant->pft(), 2, Parameters::params.stabilization);
    }

    //2. distribute resources
    for (auto const& plant_ptr : BelowPlantList)
    {
        auto plant = plant_ptr.lock();

        comp_c = plant->comp_coef(2, symm) * prop_res(plant->pft(), 2, Parameters::params.stabilization);
        plant->Buptake += BResConc * comp_c / comp_tot;
    }

    bComp_weekly = comp_tot;
}

//---------------------------------------------------------------------------

double Cell::prop_res(const string& type, const int layer, const int version) const
{
    switch (version)
    {
    case 0:
        return 1;
        break;
    case 1:
        if (layer == 1)
        {
            map<string, int>::const_iterator noa = PftNIndA.find(type);
            if (noa != PftNIndA.end())
            {
                return 1.0 / std::sqrt(noa->second);
            }
        }
        else if (layer == 2)
        {
            map<string, int>::const_iterator nob = PftNIndB.find(type);
            if (nob != PftNIndB.end())
            {
                return 1.0 / std::sqrt(nob->second);
            }
        }
        break;
    case 2:
        if (layer == 1)
        {
            return PftNIndA.size() / (1.0 + PftNIndA.size());
        }
        else if (layer == 2)
        {
            return PftNIndB.size() / (1.0 + PftNIndB.size());
        }
        break;
    default:
        cerr << "CCell::prop_res() - wrong input";
        exit(3);
        break;
    }
    return -1;
}
#endif

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
        comp_c = (*pi)->comp_coef(1, 2);
        (*pi)->Auptake += AResConc * comp_c / comp_tot;
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
        (*pi)->Buptake += BResConc * comp_c / comp_tot;
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
        comp_c = (*pi)->comp_coef(1, 2) * prop_res_above((*pi)->pft());
        (*pi)->Auptake += (AResConc * comp_c / comp_tot);
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
        (*pi)->Buptake += BResConc * comp_c / comp_tot;
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
