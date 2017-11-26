#ifndef MYCORRHIZA_H
#define MYCORRHIZA_H

#include <map>
#include <vector>

class Plant;

class CMycorrhiza
{
public:
    CMycorrhiza();
public:
    //
    //  This is the pool of plant that are connected to the mycorrhiza
    std::multimap<double, Plant*> GoodPool;
    //
    //  This is the pool of plants available to the mycorrhiza
    std::vector<Plant*>           BadPool;
};

#endif // MYCORRHIZA_H
