
#ifndef SRC_GENET_H_
#define SRC_GENET_H_

#include <vector>

#include "Plant.h"

class Plant;

/*
 * The super-individual that is one clonal plant.
 * Some clonal species are able to share resources.
 */
class Genet
{

public:
   static int staticID;
   int genetID;
   std::vector< Plant* > RametList;

   Genet():genetID(++staticID) { }

   void ResshareA();     // share above-ground resources
   void ResshareB();     // share below-ground resources

};

#endif
