
#ifndef SRC_GRIDENVIR_H_
#define SRC_GRIDENVIR_H_

#include <string>
#include <map>

#include "pft_struct.h"

class GridEnvir: public Grid, public CThread
{

public:

	GridEnvir();

	void InitRun();
	void OneYear();   // runs one year in default mode
	void OneRun();    // runs one simulation run in default mode
	void OneWeek();   // calls all weekly processes

	void InitInds();

	bool exitConditions();

	void SeedRain();  // distribute seeds on the grid each year
private:
    void print_param(); // prints general parameterization data
    void print_srv_and_PFT(const std::vector< Plant* > & PlantList); 	// prints PFT data
    void buildPFT_map(const std::vector< Plant* > & PlantList);
    void print_trait(); // prints the traits of each PFT
    void print_ind(const std::vector< Plant* > & PlantList); 			// prints individual data
    void print_aggregated(const std::vector< Plant* > & PlantList);		// prints longitudinal data that's not just each PFT
    void OutputGamma();
    double calculateBrayCurtis(const std::map<std::string, PFT_struct> & _PFT_map, int benchmarkYear, int theYear); // Bray-Curtis only makes sense with catastrophic disturbances

    std::vector<double> TotalShootmass;
    std::vector<double> TotalRootmass;
    std::vector<double> TotalNonClonalPlants;
    std::vector<double> TotalClonalPlants;
    std::vector<double> TotalAboveComp;
    std::vector<double> TotalBelowComp;
    std::map<std::string, int> BC_predisturbance_Pop;

public:

    static pthread_mutex_t gammalock;
    static pthread_mutex_t aggregated_lock;

};

#endif /* CGRIDENVIR_H_ */
