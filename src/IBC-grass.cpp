#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <cassert>
#include <fstream>
#include "CThread.h"
#include "itv_mode.h"
#include "Plant.h"
#include "mycorrhiza.h"
#include "Grid.h"
#include "Output.h"
#include "GridEnvir.h"
#include "Parameters.h"
#include "CSimulation.h"
#include "RandomGenerator.h"
//
//  selection of the namespace makes things easier in the code.
using namespace std;
//
//  This is the size of one side of a square area.
//  The whole simulation area is 173*173 cells.
//  It may be overridden from the command line.
long   GridSize = 173;   //  Side length in cm
//
//  This is the number if seeds to be put into the grid per PFT.
//  It can be overridden for all runs from the command line.
long no_init_seeds = 10;

int    startseed  = -1;
int    linetoexec = -1;
int    proctoexec =  1;

std::set<int> linestoexec;

#define DEFAULT_SIMFILE   "data/in/SimFile.txt"
#define DEFAULT_OUTPREFIX "default"

std::ofstream resfile;

std::string NameSimFile = DEFAULT_SIMFILE; 	  // file with simulation scenarios
std::string outputPrefix = DEFAULT_OUTPREFIX;
//
//  flag for switch off mycorrhiza simulation.
bool myc_off = false;
//
//  Turn off the ratio feature.
bool pft_ratio_off = false;
bool neutral       = false;
bool output_images = false;
bool timing        = false;
//
//  Number of PFTs in the pool of pfts.
long PFTCount = -1;
//
//  Name of a configuration file.
string configfilename;
//
//  The Randomnumber generation is something that all threads need
RandomGenerator rng;
//
//  Doing the output into the various files is something that all threads need.
Output output;
//
//  This is the mutex to do the management of the running simulations.
pthread_mutex_t waitmutex;
pthread_cond_t  wait;
//
//  This is a lock to prevent problems while writing some console output.
pthread_spinlock_t cout_lock;
//   Support functions for program parameters
//
//   This is the usage dumper to the console.
static void dump_help() {
    cerr << "usage:\n"
            "\tibc <options> <simfilename> <outputprefix>\n"
            "\t\t-h/--help       : print this usage information\n"
            "\t\t-c              : use this file with configuration data\n"
            "\t\t-n              : line to execute in simulation\n"
            "\t\t-p              : number of processors to use\n"
            "\t\t-s              : set a starting seed for random number generators\n"
            "\t\t--myc-off       : switch mycorrhiza feedback off\n"
            "\t\t--neutral       : switch the PFT to neutral mode.\n"
            "\t\t--pft-ratio-off : do not take ratio between PFTs with specific mycStat into account\n"
            "\t\t--pft-count     : number of PFTs in pool\n"
            "\t\t--init-seed     : number of seeds per PFT on init\n"
            "\t\t--output-images : create raw-images\n"
            "\t\t--timing        : tells us how long to compute for a year\n"
            "\t\t--grid-size     : size of the simulated area in cm\n";


    exit(0);

}
//
//
//  This is the processing function for long parameters.
//  It splits the argument at the equal sign into name and value string
static void process_long_parameter(string aLongParameter) {
    std::string name=aLongParameter.substr(0, aLongParameter.find_first_of('='));
    std::string value=aLongParameter.substr(aLongParameter.find_first_of('=')+1);
    //
    //  At this point we have the parameter name and its optional value as text.
    //  Afterwards the processing of a single parameter can be found be a chain
    //  of if () else if() else if() .... statements.
    //  The name of the parameter decides whether the value is needed or not.
    if (name == "help") {        
        dump_help();
    } else if (name == "myc-off"){
        myc_off = true;
    } else if (name == "neutral"){
        neutral = true;
    } else if (name == "output-images"){
        output_images = true;
    } else if (name == "pft-ratio-off"){
        pft_ratio_off = true;
    } else if (name == "timing"){
        timing = true;
    } else if (name == "pft-count") {
        PFTCount = strtol(value.c_str(), 0, 0);
    } else if (name == "init-seed") {
        no_init_seeds = strtol(value.c_str(), 0, 0);
    } else if (name == "grid-size") {
        GridSize = strtol(value.c_str(), 0, 0);
    } else {
        std::cerr << "unknown parameter : " << name << "\n";
    }
}
//
//  Because the constructor already sets the default filename we check if the name has its default content
//  and overwrite it. This is like a statemachine using the state of the filenames as state variable.
void ProcessArgs(std::string aArg) {
    if (NameSimFile == DEFAULT_SIMFILE) {
        NameSimFile = aArg;
    } else if (outputPrefix == DEFAULT_OUTPREFIX) {
        outputPrefix = aArg;
    } else {
        std::cerr << "Do not except more than two file names.\nTake a look at the help with -h or --help\n";
    }
}


static void filllinestoexec(char *p) {
    long v;
    char* e;

    do {
        v = strtol(p, &e, 10);
        if (e != 0) {
            if (*e=='\0') {
                linestoexec.insert(v);
            } else if (*e == '-') {
                p = e+1;
                long end = strtol(p, &e, 10);

                for (long i=v; i<=end; ++i) {
                    linestoexec.insert(i);
                }

            } else if (*e == ',') {
                e++;
                linestoexec.insert(v);
            }
        } else {
            linestoexec.insert(v);
        }
        p = e;
    } while ((e != 0) && (*e != '\0'));
}

int main(int argc, char* argv[])
{
    //
    //
    //  Beginning of a new parameter parser.
    //
    //
    int   i = 1;
    /*
      * Parse the program parameters.
      */
     while ((i<argc) && (argv[i]!=0)) {
         char *s=argv[i];

         if (*s=='-') {
             s++;
             switch (*s) {
             case 's':
                 s++;
                 if (*s!='\0') {
                     startseed=atoi(s);
                 } else {
                     i++;
                     s=argv[i];
                     if (s!=0) {
                         startseed=atoi(s);
                     } else {
                     }
                 }
                 break;
             case 'n':
                 s++;
                 if (*s!='\0') {
                 } else {
                     i++;
                     s=argv[i];
                 }
                 filllinestoexec(s);
                 break;
             case 'p':
                 s++;
                 if (*s!='\0') {
                     proctoexec=atoi(s);
                 } else {
                     i++;
                     s=argv[i];
                     if (s!=0) {
                         proctoexec=atoi(s);
                     } else {
                     }
                 }
                 break;
             case '-':    //  Long parameter
                 s++;     //  Move on to parameter name
                 process_long_parameter(s);
                 break;
             case 'c':
                 s++;
                 if (*s!='\0') {
                     configfilename=s;
                 } else {
                     i++;
                     s=argv[i];
                     if (s!=0) {
                         configfilename=s;
                     } else {
                     }
                 }
                 break;
             case 'h':
                 dump_help();
                 break;
             default:
                 std::cerr << "Unknown parameter. See usage\n";
                 dump_help();
                 break;
             }
         } else {
             ProcessArgs(argv[i]);
         }
         i++;
     }
    std::cerr << "Using simfile : " << NameSimFile << endl << "Using output prefix : " << outputPrefix << endl;
    std::cerr << "Grid-Size is " << GridSize << "cm for all runs. Starting with ";
    if (PFTCount == -1) {
        std::cerr << "all";
    } else {
        std::cerr << PFTCount;
    }

    //resfile.open("resfile.csv");
    std::cerr << " PFTs from the pool of PFTs\n";
    std::cerr << "Initial seeding uses " << no_init_seeds << " of each PFT used\n";
    //
    //  This is the end of a new program parameter parser.
    //
    //
    ifstream SimFile(NameSimFile.c_str()); // Open simulation parameterization file
    int      _NRep;
    int      linecounter = 1;
	// Temporary strings
    string   trash;
    string   data;
    //
    //  Check if the simfile could be opened. If not, give a error message
    if (SimFile.good()) {
        //
        // initialize the lock in the simulation class
        pthread_mutex_init(&GridEnvir::gammalock, 0);
        pthread_mutex_init(&GridEnvir::aggregated_lock, 0);
        //
        //  Initialize the output lock
        pthread_spin_init(&cout_lock, PTHREAD_PROCESS_PRIVATE);
        //
        //  Make it crazy save.
        if (getline(SimFile, data).good()) {
            std::stringstream ss(data);
            ss >> trash >> _NRep; 		            // Remove "NRep" header, set NRep
            if (getline(SimFile, trash).good()) { 	// Remove parameterization header file
                //
                //  We expect to run a simulation and doing the initialization of our sync
                //  variables.
                pthread_mutex_init(&waitmutex, 0);
                pthread_cond_init(&wait, 0);
                //
                //  As long as we get more lines from the simfile we let them run.
                //  Each simulation done as often as requested by the NRep value.
                while (getline(SimFile, data).good())
                {
                    //
                    //  Here we check whether the requested line matches and if this line starts with
                    //  the comment character. Lines starting with '#' are comments are not executed.
                    if (((linestoexec.empty()) || (linestoexec.find(linecounter) != linestoexec.end())) && (data[0] !='#')) {
                        for (int i = 0; i < _NRep; i++)
                        {
                            //
                            //  Create a new run.
                            CSimulation* run = new CSimulation(i, data) ;
                            //
                            //  Start the new run.
                            run->Create();
                            pthread_mutex_lock(&waitmutex);
                            //
                            //  Only if the requested number of parallel threads is reached we wait here
                            //  for a thread to complete
                            if (CThread::Count == proctoexec) {
                                pthread_cond_wait(&wait, &waitmutex);
                            }
                            pthread_mutex_unlock(&waitmutex);
                        }
                    }
                    //
                    //  We do count comment-lines as well.
                    //  If in need to start a specific line, take your favorite
                    //  editor and let it show the line.
                    linecounter++;
                }
                //
                //  We are done with all lines and repetition.
                //  But we must wait until the last thread has been terminated.
                pthread_mutex_lock(&waitmutex);
                //
                //  Only if the requested number of parallel threads is reached we wait here
                //  for a thread to complete
                while (CThread::Count > 0) {
                    pthread_cond_wait(&wait, &waitmutex);
                }
                pthread_mutex_unlock(&waitmutex);
            } else {
                cerr << "Could not read the header from " << NameSimFile << std::endl;
            }
        } else {
            cerr << "Could no read the first line from " << NameSimFile << std::endl;
        }
        SimFile.close();
    } else {
        cerr << "Could not open file " << NameSimFile << std::endl;
    }
	return 0;
}

void ThreadDone(void) {
    pthread_mutex_lock(&waitmutex);
    pthread_cond_broadcast(&wait);
    pthread_mutex_unlock(&waitmutex);
}
