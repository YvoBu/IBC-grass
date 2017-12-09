#include <iostream>
#include "mycorrhiza.h"
#include "itv_mode.h"
#include "Plant.h"
#include "RandomGenerator.h"

#define MAXPLANTS_FROMBADPOOL  0.75
#define GOODPOOL_HELPFULLLIMIT 0.25

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
        std::cerr << "Something wrong. Try to add a plant to the mycorrhiza that is already there\n";
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
        PtrPool.erase(aPlant);
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
        std::map<Plant*, PlantInfo>::iterator mp;

        mp = GoodPool.find(aPlant);
        if (mp != GoodPool.end()) {
            GoodPool.erase(mp);
            return;
        }
        //
        //  If we got here the plant could not be found in the pools. Thats bad.
        std::cerr << "Something is wrong. Plant not in pools.\n";
    } else {
        std::cerr << "Something is wrong. Try to remove an unknown plant from the mycorrhiza\n";
    }
}

void CMycorrhiza::Detach(Plant* aPlant) {
    //
    //  First check if we are attached.
    std::map<Plant*, PlantInfo>::iterator pi = GoodPool.find(aPlant);

    if (pi != GoodPool.end()) {
        //
        //  After that it is easy. Remove from good pool, add to BadPool.
        GoodPool.erase(pi);
        BadPool.push_back(aPlant);
    }
}

void CMycorrhiza::UpdatePool() {
    //
    //  We check if the good pool is empty. So we need to initialize.
    //  Even if the simulation runs out of plants this is no problem because there
    //  are no plants in the GoodPool to do any evaluation.
    if (GoodPool.empty()) {
        //
        //  We take most percent of the badpool into the good pool.
        int count = BadPool.size()*MAXPLANTS_FROMBADPOOL;
        //
        //  Loop over the initial count.
        for (int i=0; i<count; ++i) {
            //
            //  Calculate a random index.
            int index = BadPool.size()*rng.get01();
            //
            //  Attach the mycorrhiza if it allows it.
            bool attached = BadPool[index]->Attach(this);

            if (attached) {
                //
                //  Take the feedback rate that is defined for specific PFT within a plant.
                PlantInfo pli{0.0, BadPool[index]->mycFbrate};
                //
                //  Move the plant into the good pool.
                GoodPool.insert(std::pair<Plant*, PlantInfo>(BadPool[index], pli));
                //
                //  remove the plant from the BadPool.
                BadPool.erase(BadPool.begin()+index);
            }
        }
    } else {
        //
        //  Evaluating what plants to dismiss.
        size_t                                plantcount = GoodPool.size()+BadPool.size();
        std::list<Plant*>                     deletionlist;
        std::map<Plant*, PlantInfo>::iterator mi;
        //
        //  Only if we got any usefull offer before we do an update.
        if (MaxOffer != 0.0) {
            //
            //  Collect all plants that must be removed.
            for (mi=GoodPool.begin(); mi != GoodPool.end(); ++mi) {
                if (mi->second.HelpOffer < (GOODPOOL_HELPFULLLIMIT*MaxOffer)) {
                    deletionlist.push_back(mi->first);
                }
            }
            //
            //  Maybe all plants are helpfull. Non got deleted.
            if (!deletionlist.empty()) {
                //
                //  Delete the plants from the good pool to keep the pool small.
                std::list<Plant*>::iterator pi;

                for (pi = deletionlist.begin(); pi != deletionlist.end(); ++pi) {
                    GoodPool.erase(*pi);
                    (*pi)->Detach();
                }
                //
                //  Do the renewal of the good pool
                int renewcount = (plantcount*MAXPLANTS_FROMBADPOOL) - GoodPool.size();

//                std::cerr << "MaxOffer: " << MaxOffer << " Delete: " << deletionlist.size() << " Renew: " << renewcount << std::endl;
                //
                //  Loop over the renew count.
                for (int i=0; (i<renewcount) && (!BadPool.empty()); ++i) {
                    //
                    //  Calculate a random index.
                    int index = BadPool.size()*rng.get01();
                    //
                    //  Attach the mycorrhiza if it allows it.
                    bool attached = BadPool[index]->Attach(this);

                    if (attached) {
                        //
                        //  Take the feedback rate that is defined for specific PFT within a plant.
                        PlantInfo pli{0.0, BadPool[index]->mycFbrate};
                        //
                        //  Move the plant into the good pool.
                        GoodPool.insert(std::pair<Plant*, PlantInfo>(BadPool[index], pli));
                        //std::cerr << "HelpFactor: " << pli.HelpFactor << std::endl;

                        //
                        //  remove the plant from the BadPool.
                        BadPool.erase(BadPool.begin()+index);
                    }
                }
                //
                //  move the dismissed plants into the badpool
                for (pi = deletionlist.begin(); pi != deletionlist.end(); ++pi) {
                    BadPool.push_back(*pi);
                }
            }
        }
    }
    //
    //  Reset the MaxOffer for the next round.
    MaxOffer = 0.0;
}

double CMycorrhiza::HelpMe(Plant* aPlant, double aResource, double aDemand) {
    double retval = 0.0;
    std::map<Plant*, PlantInfo>::iterator mp;
    //
    //  Search the plant to help or not
    mp=GoodPool.find(aPlant);
    if (mp != GoodPool.end()) {
        //
        //  Update the MaxOffer to decide later what plants are no help for the
        //  mycorrhiza.
        if (aResource > MaxOffer) {
            MaxOffer = aResource;
        }
        //
        //  Store the last resource offer from the plant for later analysis.
        mp->second.HelpOffer=aResource;
        //
        //  Calculate the amount of resource that the mycorrhiza returns.
        retval = (1+mp->second.HelpFactor) * aDemand;
//        std::cerr << "Help:" << retval << " aResource:" << aResource << std::endl;
    } else {
        std::cerr << "Something is wrong. Unknown plant ask for help.\n";
    }
    return retval;
}
