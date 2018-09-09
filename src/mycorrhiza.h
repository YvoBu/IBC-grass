#ifndef MYCORRHIZA_H
#define MYCORRHIZA_H

#include <map>
#include <vector>
#include <set>
#include <list>

class Plant;

class CMycorrhiza
{
public:
    CMycorrhiza();
    void Add(Plant* aPlant);
    //
    // dead plant is removed from all lists
    void Remove(Plant* aPlant);
    //
    // plant dismisses mycorrhiza
    void Detach(Plant* aPlant);
    void UpdatePool(void);
    double HelpMe(Plant* aPlant, double aResource, double aDemand);
private:
    struct PlantInfo {
        double HelpOffer;
        double HelpFactor;
    };

    //
    //  This is the pool of plant that are connected to the mycorrhiza
    std::map<Plant*, PlantInfo> GoodPool;
    //
    //  This is the pool of plants available to reconnect with the mycorrhiza
    std::vector<Plant*>      BadPool;
    //
    //  This is a set of pointers to plants to verify pointers passed to this
    //  object.
    std::set<Plant*>         PtrPool;
    //
    //  This is the maximum offered resource by a plant.
    double MaxOffer;
};

#endif // MYCORRHIZA_H
