#ifndef IBCGRASS_H
#define IBCGRASS_H

#include <string>

#define getGridArea() GridSize*GridSize

extern long GridSize;

extern std::string NameSimFile;
extern std::string outputPrefix;
extern Output output;

void ThreadDone(void);
#endif // IBCGRASS_H
