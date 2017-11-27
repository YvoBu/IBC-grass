
#include <cassert>
#include <iostream>
#include <vector>

#include "itv_mode.h"
#include "Traits.h"
#include "Seed.h"
#include "Environment.h"
#include "Plant.h"

using namespace std;

//-----------------------------------------------------------------------------

/*
 * Constructor for normal reproduction
 */
Seed::Seed(const Traits & t, Cell* _cell, ITV_mode itv, double aSD) : Traits(t),
		cell(NULL),
		age(0), toBeRemoved(false)
{
    if (itv == on) {
        varyTraits(aSD);
	}
    pSeedEstab = pEstab;
    mass = seedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}

//-----------------------------------------------------------------------------

/*
 * Constructor for initial establishment (with germination pre-set)
 */
Seed::Seed(const Traits & t, Cell*_cell, double new_estab, ITV_mode itv, double aSD) : Traits(t),
		cell(NULL),
		age(0), toBeRemoved(false)
{
    if (itv == on) {
        varyTraits(aSD);
	}

    pSeedEstab = new_estab;
    mass = seedMass;

	assert(this->cell == NULL);
	this->cell = _cell;
}
