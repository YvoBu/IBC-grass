#include <cassert>
#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>

#include "itv_mode.h"
#include "Traits.h"
#include "Environment.h"

#include "RandomGenerator.h"

using namespace std;
extern RandomGenerator rng;

/*
 * Default constructor
 */
Traits::Traits() :
        myTraitType(Traits::species), PFT_ID("EMPTY"),
        LMR(-1), SLA(-1), RAR(1), m0(-1), maxMass(-1),
        allocSeed(0.05), seedMass(-1), dispersalDist(-1), dormancy(1), pEstab(0.5),
        Gmax(-1), palat(-1), memory(-1),
        mThres(0.2), growth(0.25), flowerWeek(16), dispersalWeek(20),
        clonal(false), meanSpacerlength(0), sdSpacerlength(0),
        allocSpacer(0), resourceShare(false), mSpacer(0), mycZOI(0.0), mycCOMP(0.0),  mycC(0.0)
{

}

Traits::Traits(std::string line) :
    myTraitType(Traits::species), PFT_ID("EMPTY"),
    LMR(-1), SLA(-1), RAR(1), m0(-1), maxMass(-1),
    allocSeed(0.05), seedMass(-1), dispersalDist(-1), dormancy(1), pEstab(0.5),
    Gmax(-1), palat(-1), memory(-1),
    mThres(0.2), growth(0.25), flowerWeek(16), dispersalWeek(20),
    clonal(false), meanSpacerlength(0), sdSpacerlength(0),
    allocSpacer(0), resourceShare(false), mSpacer(0), mycZOI(0.0), mycCOMP(0.0), mycC(0.0)
{
    std::stringstream ss(line);
    double mycCin;

    ss >> PFT_ID
       >> allocSeed >> LMR >> m0
        >> maxMass >> seedMass >> dispersalDist
        >> pEstab >> Gmax >> SLA
        >> palat >> memory >> RAR
        >> growth >> mThres >> clonal
        >> meanSpacerlength >> sdSpacerlength >> resourceShare
        >> allocSpacer
        >> mSpacer >> mycStat;
    /*
     * Checking the input stream for more values.
     * Only if the stream is still good we read in more.
     */
    if (ss.good()) {
        ss >> mycZOI;
        if (!ss.good()) {
            if (mycStat == "OM") {
                mycZOI = ((rng.rng() |0x01u) / ((double) UINT32_MAX)) + 1.0; // generates random number between 1.0 (0x01u) and 2.0 (+1.0)
            } else if (mycStat == "FM") {
                mycZOI = (rng.rng() / ((double) UINT32_MAX)) + 1.0; // generates random number between or equal to 1.0 and 2.0
            } else if (mycStat == "NM") {
                mycZOI = 1.0;
            }
        }
    } else {
        if (!ss.good()) {
            if (mycStat == "OM") {
                mycZOI = ((rng.rng() |0x01u) / ((double) UINT32_MAX)) + 1.0; // generates random number between 1.0 (0x01u) and 2.0 (+1.0)
            } else if (mycStat == "FM") {
                mycZOI = (rng.rng() / ((double) UINT32_MAX)) + 1.0; // generates random number between or equal to 1.0 and 2.0
            } else if (mycStat == "NM") {
                mycZOI = 1.0;
            }
        }
    }
    if (ss.good()) {
        ss >> mycCOMP;
    }
    if (!ss.good()) {
        if (mycStat == "OM") {
            mycCOMP = ((rng.rng() |0x01u) / ((double) UINT32_MAX) / 2.0); // generates random number between 0 (0x01u) and 2.0
        } else if (mycStat == "FM") {
            mycCOMP = (rng.rng() / ((double) UINT32_MAX)) + 1.0; // generates random number between or equal to 1.0 and 2.0
        } else if (mycStat == "NM") {
            mycCOMP = 1.0;
        }
    }
    if (ss.good()) {
        ss >> mycCin;
        mycC = mycCin/100.0;
    }
    if (!ss.good()) {
        if (mycStat == "OM") {
            mycC = (rng.rng() / (((double) UINT32_MAX ) / 0.4)) + 0.1; // generates random number between 0.1 and 0.5
        } else if (mycStat == "FM") {
            mycC = (rng.rng() / (((double) UINT32_MAX) / 0.1)) + 0.1; // generates random number between 0.1 and 0.2
        } else if (mycStat == "NM") {
            mycC = 0;
        }
    }
    // optimization for maxMass calculation
    maxMassPow_4_3rd = pow(maxMass, (4.0/3.0));
}

/**
 * Retrieve a deep-copy some arbitrary trait set (for plants dropping seeds)
 */
unique_ptr<Traits> Traits::copyTraitSet(const unique_ptr<Traits> & t)
{
    return (make_unique<Traits>(*t));
}

//-----------------------------------------------------------------------------
/**
 * Read definition of PFTs used in the simulation
 * @param file file containing PFT definitions
 */

/* MSC
 * Vary the current individual's traits, based on a Gaussian distribution with a
 * standard deviation of "ITVsd". Sub-traits that are tied will vary accordingly.
 * Bounds on 1 and -1 ensure that no trait garners a negative value and keep the resulting
 * distribution balanced. Other, trait-specific, requirements are checked as well. (e.g.,
 * LMR cannot be greater than 1, memory cannot be less than 1).
 */
void Traits::varyTraits(double aSD)
{

    assert(myTraitType == Traits::species);

    myTraitType = Traits::individualized;
    double dev;

    double LMR_;
    do
    {
        dev = rng.getGaussian(0, aSD);
        LMR_ = LMR + (LMR * dev);
    } while (dev < -1.0 || dev > 1.0 || LMR_ < 0 || LMR_ > 1);
    LMR = LMR_;

    double m0_, MaxMass_, SeedMass_, Dist_;
    do
    {
        dev = rng.getGaussian(0, aSD);
        m0_ = m0 + (m0 * dev);
        MaxMass_ = maxMass + (maxMass * dev);
        SeedMass_ = seedMass + (seedMass * dev);
        Dist_ = dispersalDist - (dispersalDist * dev);
    } while (dev < -1.0 || dev > 1.0 || m0_ < 0 || MaxMass_ < 0 || SeedMass_ < 0 || Dist_ < 0);
    m0 = m0_;
    maxMass = MaxMass_;
    seedMass = SeedMass_;
    dispersalDist = Dist_;

    double Gmax_;
    int memory_;
    do
    {
        dev = rng.getGaussian(0, aSD);
        Gmax_ = Gmax + (Gmax * dev);
        memory_ = memory - (memory * dev);
    } while (dev < -1.0 || dev > 1.0 || Gmax_ < 0 || memory_ < 1);
    Gmax = Gmax_;
    memory = memory_;

    double palat_, SLA_;
    do
    {
        dev = rng.getGaussian(0, aSD);
        palat_ = palat + (palat * dev);
        SLA_ = SLA + (SLA * dev);
    } while (dev < -1.0 || dev > 1.0 || palat_ < 0 || SLA_ < 0);
    palat = palat_;
    SLA = SLA_;

    double meanSpacerlength_, sdSpacerlength_;
    do
    {
        dev = rng.getGaussian(0, aSD);
        meanSpacerlength_ = meanSpacerlength + (meanSpacerlength * dev);
        sdSpacerlength_ = sdSpacerlength + (sdSpacerlength * dev);
    } while (dev < -1.0 || dev > 1.0 || meanSpacerlength_ < 0 || sdSpacerlength_ < 0);
    meanSpacerlength = meanSpacerlength_;
    sdSpacerlength = sdSpacerlength_;

}
