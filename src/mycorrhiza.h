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
    void Remove(Plant* aPlant);
    void UpdatePool(void);
    double HelpMe(double aResource);
private:
    //
    //  This is the pool of plant that are connected to the mycorrhiza
    std::list<Plant*>   GoodPool;
    //
    //  This is the pool of plants available to the mycorrhiza
    std::vector<Plant*> BadPool;
    //
    //  This is a set of pointers to plants to verify pointers passed to this
    //  object.
    std::set<Plant*>    PtrPool;

};

#endif // MYCORRHIZA_H
