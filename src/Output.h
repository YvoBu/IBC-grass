#ifndef SRC_OUTPUT_H_
#define SRC_OUTPUT_H_

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "pft_struct.h"

class Output
{

public:
    // Describes simulation parameters. These are static throughout and provided at initialization.
    static const std::vector<std::string> param_header;

    // Describes the traits of each PFT. These are static and provided at initialization.
    static const std::vector<std::string> trait_header;

    // Describes the extinction times or final populations of PFTs.
    static const std::vector<std::string> srv_header;

    // Describes PFT's measured variables.
    static const std::vector<std::string> PFT_header;

    // Describes individual level variables over time.
    static const std::vector<std::string> ind_header;

    // Environmental and incidental data collection
    static const std::vector<std::string> aggregated_header;

    // Filenames
    std::string param_fn;
    std::string trait_fn;
    std::string srv_fn;
    std::string PFT_fn;
    std::string ind_fn;

    bool is_file_exist(const char *fileName);

public:

    Output();
    ~Output();

    void setupOutput(std::string param_fn, std::string trait_fn, std::string srv_fn, std::string PFT_fn, std::string ind_fn);
    void cleanup();

//    void print_param(); // prints general parameterization data

    double calculateShannon(const std::map<std::string, PFT_struct> & _PFT_map);
    double calculateRichness(const std::map<std::string, PFT_struct> & _PFT_map);
    double calculateBrayCurtis(const std::map<std::string, PFT_struct> & _PFT_map, int benchmarkYear, int theYear); // Bray-Curtis only makes sense with catastrophic disturbances
    std::map<std::string, double> calculateMeanTraits(const std::vector< Plant* > & PlantList);

    void print_row(std::ostringstream &ss, std::ofstream &stream);
    void print_row(std::vector<std::string> row, std::ofstream &stream);

    // aggregated output

    std::ofstream param_stream;
    std::ofstream trait_stream;
    std::ofstream srv_stream;
    std::ofstream PFT_stream;
    std::ofstream ind_stream;
    std::ofstream aggregated_stream;

    pthread_mutex_t outputlock;
    pthread_mutex_t openlock;
};

#endif /* SRC_OUTPUT_H_ */
