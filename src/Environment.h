#ifndef SRC_ENVIRONMENT_H_
#define SRC_ENVIRONMENT_H_

#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include "Parameters.h"
#include "RandomGenerator.h"
//
//  This class should hold all data that are specific to the environment that is
//  simulated.
class Environment : public Parameters
{

public:
    std::vector<std::string> PftInitList; 	// list of Pfts used
    std::map<std::string, int> PftSurvTime;	// array for survival times of PFTs (in years);

	const static int WeeksPerYear;  // number of weeks per year (constantly at value 30)

    int week;	// current week (1-30)
    int year;    // current year

    int SimID;   // simulation-ID
    int ComNr;	// Community identifier for multiple parameter settings of the same community.
    int RunNr;   // repetition number

    double mycFbRange;
    double mycFbOffset;

	Environment();
	~Environment();

    void GetSim(std::string data); 	// Simulation read in

	/*
	 * Helper function for comparing floating point numbers for equality
	 */
	inline static bool AreSame(double const a, double const b) {
	    return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
	}
    std::string getSimID(); // Merge ID for data sets
    void ReadPFTDef(const std::string& file);
    std::map< std::string, Traits >      pftTraitTemplates; // links of PFTs (Traits) used
    std::vector< std::string>            pftInsertionOrder;
};

#endif
