
#include <cassert>
#include <iostream>

#include "itv_mode.h"
#include "Cell.h"
#include "Seed.h"
#include "Plant.h"
#include "Environment.h"
#include "Traits.h"
#include "RandomGenerator.h"
#include "Output.h"
#include "IBC-grass.h"

#include "mycorrhiza.h"

using namespace std;

extern RandomGenerator rng;
extern bool myc_off;
//-----------------------------------------------------------------------------
/**
 * constructor - germination
 *
 * If a seed germinates, the new plant inherits its parameters.
 * Genet has to be defined externally.
 */

int Plant::staticID = 0;

Plant::Plant(const Seed & seed, ITV_mode itv) : Traits(seed),
		cell(NULL), mReproRamets(0), genet(),
		plantID(++staticID), x(0), y(0),
		age(0), mRepro(0), Ash_disc(0), Art_disc(0), Auptake(0), Buptake(0),
        isStressed(0), isDead(false), toBeRemoved(false)
{
    myc = 0;
    spacerLengthToGrow = 0;

    if (itv == on) {
        assert(myTraitType == Traits::individualized);
	} else {
        assert(myTraitType == Traits::species);
	}

    mShoot = m0;
    mRoot = m0;

	//establish this plant on cell
    setCell(seed.getCell());
	if (cell)
	{
		x = cell->x;
		y = cell->y;
	}
}

//-----------------------------------------------------------------------------
/**
 * Clonal Growth - The new Plant inherits its parameters from 'plant'.
 * Genet is the same as for plant
 */
Plant::Plant(double x, double y, const Plant* plant, ITV_mode itv) : Traits(*plant),
		cell(NULL), mReproRamets(0), genet(plant->genet),
		plantID(++staticID), x(x), y(y),
		age(0), mRepro(0), Ash_disc(0), Art_disc(0), Auptake(0), Buptake(0),
        isStressed(0), isDead(false), toBeRemoved(false)
{
    myc = 0;
    spacerLengthToGrow = 0;

    if (itv == on) {
        assert(myTraitType == Traits::individualized);
	} else {
        assert(myTraitType == Traits::species);
	}

    mShoot = m0;
    mRoot = m0;
}

//---------------------------------------------------------------------------

Plant::~Plant()
{
    //
    //  Because the growing spacers are not part of the plantlist we need to
    //  delete the spacers if the plant gets deleted.
    std::vector<Plant*>::iterator pi;

    for (pi = growingSpacerList.begin(); pi != growingSpacerList.end(); ++pi) {
        delete *pi;
    }
}

void Plant::weeklyReset()
{
	Auptake = 0;
	Buptake = 0;
	Ash_disc = 0;
	Art_disc = 0;
}

//-----------------------------------------------------------------------------

void Plant::setCell(Cell* _cell) {
	assert(this->cell == NULL && _cell != NULL);

	this->cell = _cell;
	this->cell->occupied = true;
}

//---------------------------------------------------------------------------
/**
 * Growth of reproductive organs (seeds and spacer).
 */
double Plant::ReproGrow(double uptake, int aWeek)
{
	double VegRes;

	//fixed Proportion of resource to seed production
    if (mRepro <= allocSeed * mShoot)
	{
        double SeedRes = uptake * allocSeed;
        double SpacerRes = uptake * allocSpacer;

		// during the seed-production-weeks
        if ((aWeek >= flowerWeek) && (aWeek < dispersalWeek))
		{
			//seed production
            double dm_seeds = max(0.0, growth * SeedRes);
			mRepro += dm_seeds;

			//clonal growth
			double d = max(0.0, min(SpacerRes, uptake - SeedRes)); // for large AllocSeed, resources may be < SpacerRes, then only take remaining resources
            mReproRamets += max(0.0, growth * d);

			VegRes = uptake - SeedRes - d;
		}
		else
		{
			VegRes = uptake - SpacerRes;
            mReproRamets += max(0.0, growth * SpacerRes);
		}

	}
	else
	{
		VegRes = uptake;
	}

	return VegRes;
}

//-----------------------------------------------------------------------------
/**
 * Growth of the spacer.
 */
void Plant::SpacerGrow() {

	if (growingSpacerList.size() == 0 || Environment::AreSame(mReproRamets, 0))
	{
		return;
	}

	double mGrowSpacer = mReproRamets / growingSpacerList.size(); //resources for one spacer

	for (auto const& Spacer : growingSpacerList)
	{
        Spacer->spacerLengthToGrow = max(0.0, Spacer->spacerLengthToGrow - (mGrowSpacer / mSpacer));
	}

	mReproRamets = 0;
}

//-----------------------------------------------------------------------------
/**
 * two-layer growth
 * -# Resources for fecundity are allocated
 * -# According to the resources allocated and the respiration needs
 * shoot- and root-growth are calculated.
 * -# Stress-value is in- or decreased according to the uptake
 *
 * adapted growth formula with correction factor for the conversion rate
 * to simulate implicit biomass reduction via root herbivory
 */
void Plant::Grow(int aWeek) //grow plant one timestep
{
	double dm_shoot, dm_root, alloc_shoot;
	double LimRes, ShootRes, RootRes, VegRes;

	/********************************************/
	/*  dm/dt = growth*(c*m^p - m^q / m_max^r)  */
	/********************************************/
    if (!myc_off) {
        //
        //  We have mycorrhiza support.
        if (myc != 0) {
            //
            //  Buptake is limiting. Ask for help.
            if (Buptake < Auptake) {
                //
                //  Calculating amount of resource to take from a-uptake
                double resoffer = Auptake * mycC;
                //
                //  Reduce a-uptake
                Auptake-=resoffer;
                //
                //  ask for help.
                double reshelp = myc->HelpMe(this, resoffer);
                //
                //  Anyway we take the help from the mycorrhiza
                Buptake+= reshelp;
                //
                //  But if we are an FM class plant we check the benefit
                if ((mycStat == "FM") && (reshelp < resoffer)) {
                    //
                    // The mycorrhiza is not doing enough for us.
                    // Dismiss!
                    myc->Detach(this);
                    myc = 0;
                }
            } else {
            }
        } else {
        }
    }
    // which resource is limiting growth?
    LimRes = min(Buptake, Auptake); // two layers

    VegRes = ReproGrow(LimRes, aWeek);
	// allocation to shoot and root growth
	alloc_shoot = Buptake / (Buptake + Auptake); // allocation coefficient

	ShootRes = alloc_shoot * VegRes;
	RootRes = VegRes - ShootRes;

	// Shoot growth
	dm_shoot = this->ShootGrow(ShootRes);

	// Root growth
	dm_root = this->RootGrow(RootRes);

	mShoot += dm_shoot;
	mRoot += dm_root;

	if (stressed())
	{
		++isStressed;
	}
	else if (isStressed > 0)
	{
		--isStressed;
	}

}

//-----------------------------------------------------------------------------
/**
 * shoot growth : dm/dt = growth*(c*m^p - m^q / m_max^r)
 */
double Plant::ShootGrow(double shres)
{

	double Assim_shoot;
	double Resp_shoot;

	// exponents for growth function
	double p = 2.0 / 3.0;
	double q = 2.0;

    Assim_shoot = growth * min(shres, Gmax * Ash_disc);                                 //growth limited by maximal resource per area -> similar to uptake limitation
    Resp_shoot = growth_SLA_Gmax * pow(LMR, p) * pow(mShoot, q) / maxMassPow_4_3rd; //respiration proportional to mshoot^2

	return max(0.0, Assim_shoot - Resp_shoot);

}

//-----------------------------------------------------------------------------
/**
 * root growth : dm/dt = growth*(c*m^p - m^q / m_max^r)
 */
double Plant::RootGrow(double rres)
{
	double Assim_root, Resp_root;

	//exponents for growth function
	double q = 2.0;

    Assim_root = growth * min(rres, Gmax * Art_disc); //growth limited by maximal resource per area -> similar to uptake limitation
    Resp_root = growth_RAR_Gmax * pow(mRoot, q) / maxMassPow_4_3rd;  //respiration proportional to root^2

	return max(0.0, Assim_root - Resp_root);
}

//-----------------------------------------------------------------------------

bool Plant::stressed() const
{
	return (Auptake / 2.0 < minresA()) || (Buptake / 2.0 < minresB());
}

//-----------------------------------------------------------------------------
/**
 * Kill plant depending on stress level and base mortality. Stochastic process.
 */
void Plant::Kill(double aBackgroundMortality)
{
    assert(memory >= 1);

    double pmort = (double(isStressed) / double(memory)) + aBackgroundMortality; // stress mortality + random background mortality
    double amort = rng.rng() / (double) UINT32_MAX;

    if (amort < pmort)
	{
		isDead = true;
	}
}

//-----------------------------------------------------------------------------
/**
 * Litter decomposition with deletion at 10mg.
 */
void Plant::DecomposeDead(double aLitterDecomp) {

	assert(isDead);

	const double minmass = 10; // mass at which dead plants are removed

	mRepro = 0;
    mShoot *= aLitterDecomp;
    mRoot *= aLitterDecomp;

	if (GetMass() < minmass)
	{
		toBeRemoved = true;
	}
}

//-----------------------------------------------------------------------------
/**
 * If the plant is alive and it is dispersal time, the function returns
 * the number of seeds produced during the last weeks.
 * Subsequently the allocated resources are reset to zero.
 */
int Plant::ConvertReproMassToSeeds()
{
	int NSeeds = 0;

	if (!isDead)
	{
        NSeeds = floor(mRepro / seedMass);

		mRepro = 0;
	}

	lifetimeFecundity += NSeeds;

	return NSeeds;
}

//-----------------------------------------------------------------------------
/**
 returns the number of new spacer to set:
 - 1 if there are enough clonal-growth resources and spacer list is empty
 - 0 otherwise
 */
int Plant::GetNRamets() const
{
	if (mReproRamets > 0 &&
			!isDead &&
			growingSpacerList.size() == 0) {
		return 1;
	}

	return 0;
}

//-----------------------------------------------------------------------------
/**
 * Remove half shoot mass and seed mass from a plant.
 */
double Plant::RemoveShootMass(double aBiteSize)
{
	double mass_removed = 0;

	if (mShoot + mRepro > 1) // only remove mass if shootmass > 1 mg
	{
        mass_removed = aBiteSize * mShoot + mRepro;
        mShoot *= 1 - aBiteSize;
		mRepro = 0;
	}

	return mass_removed;
}

//-----------------------------------------------------------------------------
/**
 * Remove root mass from a plant.
 */
void Plant::RemoveRootMass(const double mass_removed)
{
	assert(mass_removed <= mRoot);

	mRoot -= mass_removed;

	if (Environment::AreSame(mRoot, 0)) {
		isDead = true;
	}
}

//-----------------------------------------------------------------------------
/**
 * Winter dieback of aboveground biomass. Aging of Plant.
 */
void Plant::WinterLoss(double aWinterDieback)
{
    mShoot *= 1 - aWinterDieback;
	mRepro = 0;
	age++;
}

//-----------------------------------------------------------------------------
/**
 * Competitive strength of plant.
 * @param layer above- (1) or belowground (2) ZOI
 * @param symmetry Symmetry of competition
 * (symmetric, partial asymmetric, complete asymmetric )
 * @return competitive strength
 */
double Plant::comp_coef(const int layer, const int symmetry) const
{
	switch (symmetry)
	{
	case 1:
		if (layer == 1)
            return Gmax;
		if (layer == 2)
            return Gmax;
		break;
	case 2:
        if (layer == 1) {
            return mShoot * CompPowerA();
        }
        if (layer == 2) {
            if (myc_off) {
                return mRoot * CompPowerB();
            } else {
                return mRoot * CompPowerB() * ((myc != 0)?mycCOMP:1.0);
            }
        }
		break;
	default:
		cerr << "CPlant::comp_coef() - wrong input";
		exit(1);
	}

	return -1;
}

double Plant::Area_root() {
    if (!myc_off) {
      return RAR * pow(mRoot, 2.0 / 3.0) * ((myc != 0)?mycZOI:1.0);
  } else {
      return RAR * pow(mRoot, 2.0 / 3.0);
  }
}
double Plant::Radius_root() {
    if (!myc_off) {
        return sqrt(RAR * ((myc != 0)?mycZOI:1.0) * pow(mRoot, 2.0 / 3.0) / Pi);
    } else {
        return sqrt(RAR * pow(mRoot, 2.0 / 3.0) / Pi);
    }
}
