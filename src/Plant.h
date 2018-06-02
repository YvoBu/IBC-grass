#ifndef SRC_PLANT_H_
#define SRC_PLANT_H_

#include <vector>
#include <cmath>
#include <math.h>
#include <string>
#include <memory>
#include <set>

#include "Genet.h"
#include "Parameters.h"
#include "Traits.h"

static const double Pi = std::atan(1) * 4;

class Seed;
class Cell;
class Genet;
class CMycorrhiza;

class Plant : public Traits
{
private:
    Cell*        cell;
    CMycorrhiza* myc;

    double ReproGrow(double uptake, int aWeek);
	double ShootGrow(double shres);
	double RootGrow(double rres);

	double mReproRamets;			// resources for ramet growth

public:
    std::weak_ptr<Genet>    genet; 		// genet of the clonal plant

	static int staticID;
	int plantID;

	int x;
	int y;

	int age;
	double mShoot;   			// shoot mass
	double mRoot;    			// root mass
	double mRepro;    			// reproductive mass (converted to a discrete number of seeds)
	int lifetimeFecundity = 0; 	// The total accumulation of seeds

	int Ash_disc; 				// discrete above-ground ZOI area
	int Art_disc; 				// discrete below-ground ZOI area

	double Auptake; 			// uptake of above-ground resource in one time step
	double Buptake; 			// uptake below-ground resource one time step

	int isStressed;     			// counter for weeks with resource stress exposure
	bool isDead;      			// plant dead or alive?
	bool toBeRemoved;    			// Should the plant be removed from the PlantList?
    bool iamDeleted;

	// Clonal
    std::vector< Plant* > growingSpacerList;	// List of growing Spacer
    Plant*                parent;               // As long as the spacer lives and is not a plant
                                                // it has a parent.
	double spacerLengthToGrow;

	// Constructors
    Plant(const Seed & seed, ITV_mode itv); 						// from a germinated seed
    Plant(double x, double y, const Plant* plant, ITV_mode itv); 	// for clonal establishment
	~Plant();

    void Grow(int aWeek); 									 // shoot-root resource allocation and plant growth in two layers
    void Kill(double);  									 // Mortality due to resource shortage or at random
    void DecomposeDead(double);     						 // calculate mass shrinkage of dead plants
    void WinterLoss(double); 							 	 // removal of aboveground biomass in winter
	bool stressed() const;							 // return true if plant is stressed
	void weeklyReset();								 // reset competitive parameters that depend on current biomass
    double RemoveShootMass(double);  						 // removal of aboveground biomass by grazing
	void RemoveRootMass(const double mass_removed);  // removal of belowground biomass by root herbivory
	double comp_coef(const int layer, const int symmetry) const; // competition coefficient for a plant (for AboveComp and BelowComp)

    inline double minresA() const { return mThres * Ash_disc * Gmax; } // lower threshold of aboveground resource uptake
    inline double minresB() const { return mThres * Art_disc * Gmax; } // lower threshold of belowground resource uptake

	inline double GetMass() const { return mShoot + mRoot + mRepro; }

	inline double getHeight(double const c_height_conversion = 6.5) const {
        return pow(mShoot / LMR, 1 / 3.0) * c_height_conversion;
	}

	inline double getBiomassAtHeight(double const height, double const c_height_conversion = 6.5) const {
        return ( (pow(height, 3) * LMR) / pow(c_height_conversion, 3) );
	}

    inline double Area_shoot()		{ return SLA * pow(LMR * mShoot, 2.0 / 3.0); } // ZOI area
    double Area_root();
    inline double Radius_shoot() 	{ return sqrt(SLA * pow(LMR * mShoot, 2.0 / 3.0) / Pi); } // ZOI radius
    double Radius_root();

	void setCell(Cell* cell);
    inline Cell* getCell() const { return cell; }

    inline std::string pft() { return this->PFT_ID; }

	inline void setGenet(std::weak_ptr<Genet> _genet) { this->genet = _genet; }
	inline std::weak_ptr<Genet> getGenet() { return genet; }

    void SpacerGrow();  			        // spacer growth
    int ConvertReproMassToSeeds(); 	// returns number of seeds of one plant individual. Clears mRepro.
    int GetNRamets() const;  		        // return number of ramets

    inline static double getPalatability(const Plant* p) {
		if (p->isDead)
		{
			return 0;
		}
        return p->mShoot * p->GrazFraction();
	}

    inline static double getShootGeometry(const Plant* p) {
        return (p->mShoot / p->LMR);
	}

	// return if plant should be removed
    inline static bool GetPlantRemove(const Plant* p) {
		return p->toBeRemoved;
	}

public:
    //
    //  If the plant is non-mycorrhizal or has no mycStat set it refuses to connect the myc.
    bool Attach(CMycorrhiza* aMyc) {if ((mycStat.empty()) || (mycStat=="NM")) {return false;} else {myc = aMyc;return true;}};
    void Detach(void) {myc = 0;};
    static pthread_mutex_t  vmtx;
    static std::set<Plant*> valid;
};

#endif
