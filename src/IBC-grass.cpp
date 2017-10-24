#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <cassert>

#include "CThread.h"
#include "itv_mode.h"
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
long   GridSize = 173;   //  Side length in cm

int    startseed  = -1;
int    linetoexec = -1;
int    proctoexec =  1;

#define DEFAULT_SIMFILE   "data/in/SimFile.txt"
#define DEFAULT_OUTPREFIX "default"

std::string NameSimFile = DEFAULT_SIMFILE; 	  // file with simulation scenarios
std::string outputPrefix = DEFAULT_OUTPREFIX;

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
//   Support functions for program parameters
//
//   This is the usage dumper to the console.
static void dump_help() {
    cerr << "usage:\n"
            "\tibc <options> <simfilename> <outputprefix>\n"
            "\t\t-h/--help : print this usage information\n"
            "\t\t-c        : use this file with configuration data\n"
            "\t\t-n        : line to execute in simulation\n"
            "\t\t-p        : number of processors to use\n"
            "\t\t-s        : set a starting seed for random number generators\n";
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
                     linetoexec=atoi(s);
                 } else {
                     i++;
                     s=argv[i];
                     if (s!=0) {
                         linetoexec=atoi(s);
                     } else {
                     }
                 }
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
    cerr << "Using simfile : " << NameSimFile << endl << "Using output prefix : " << outputPrefix << endl;
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
                    if ((linetoexec == -1) || (linecounter == linetoexec)) {
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
