#include <iostream>
#include "mycorrhiza.h"
#include "itv_mode.h"
#include "Plant.h"
#include "RandomGenerator.h"

extern RandomGenerator rng;

CMycorrhiza::CMycorrhiza()
{

}

void CMycorrhiza::Add(Plant *aPlant) {
    //
    //  First check if this pointer is already part of the mycorrhiza
    std::set<Plant*>::iterator ptr = PtrPool.find(aPlant);
    //
    //  If not here we add it to the ptr-pool
    if (ptr == PtrPool.end()) {
        PtrPool.insert(aPlant);
        //
        //  The new plant gets put into the badpool.
        BadPool.push_back(aPlant);
    } else {
        //std::cerr << "Something wrong. Try to add a plant to the mycorrhiza that is already there\n";
    }
}

void CMycorrhiza::Remove(Plant *aPlant) {
    //
    //  First check if we have the pointer
    std::set<Plant*>::iterator ptr = PtrPool.find(aPlant);
    //
    //  If not here we add it to the ptr-pool
    if (ptr != PtrPool.end()) {
        //
        //  Remove it from the ptr-pool
        PtrPool.erase(ptr);
        //
        //  Check the badpool.
        std::vector<Plant*>::iterator vp;

        for (vp = BadPool.begin(); vp != BadPool.end(); ++vp) {
            //
            //  If we find the plant, remove it and exit.
            if ((*vp) == aPlant) {
                BadPool.erase(vp);
                return;
            }
        }
        //
        //  If we got here we do not find the plant in the badpool
        std::list<Plant*>::iterator mp;

        for (mp = GoodPool.begin(); mp != GoodPool.end(); ++mp) {
            if ((*mp) == aPlant) {
                GoodPool.erase(mp);
                return;
            }
        }
        //
        //  If we got here the plant could not be found in the pools. Thats bad.
        std::cerr << "Something is wrong. Plant not in pools.\n";
    } else {
        std::cerr << "Something is wrong. Try to remove an unknown plant from the mycorrhiza\n";
    }
}

void CMycorrhiza::UpdatePool() {
    //
    //  We check if the good pool is empty. So we need to initialize.
    if (GoodPool.empty()) {
        //
        //  We take 75percent of the badpool into the good pool.
        int count = BadPool.size()*0.75;
        //
        //  Loop over the initial count.
        for (int i=0; i<count; ++i) {
            //
            //  Calculate a random index.
            int index = BadPool.size()*rng.get01();
            //
            //  Attach the mycorrhiza
            BadPool[index]->Attach(this);
            //
            //  Move the plant into the good pool.
            GoodPool.push_back(BadPool[index]);
            //
            //  remove the plant from the BadPool.
            BadPool.erase(BadPool.begin()+index);
        }
    } else {

    }
}

double CMycorrhiza::HelpMe(double aResource) {
    double retval = 0.0;

    return retval;
}
