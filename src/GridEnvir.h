
#ifndef SRC_GRIDENVIR_H_
#define SRC_GRIDENVIR_H_

#include <string>

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
    void print_srv_and_PFT(const std::vector< std::shared_ptr<Plant> > & PlantList); 	// prints PFT data
    std::map<std::string, PFT_struct> buildPFT_map(const std::vector< std::shared_ptr<Plant> > & PlantList);
    void print_trait(); // prints the traits of each PFT
    void print_ind(const std::vector< std::shared_ptr<Plant> > & PlantList); 			// prints individual data
    void print_aggregated(const std::vector< std::shared_ptr<Plant> > & PlantList);		// prints longitudinal data that's not just each PFT
    void OutputGamma();
public:
    static pthread_mutex_t gammalock;
};

#endif /* CGRIDENVIR_H_ */
