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
    void SetFeedbackRange(double aMin, double aMax) { FbMin = aMin; FbMax = aMax; FbRange = FbMax-FbMin;FbOffset=FbMin;};
    void Add(Plant* aPlant);
    void Remove(Plant* aPlant);
    void Detach(Plant* aPlant);
    void UpdatePool(void);
    double HelpMe(Plant* aPlant, double aResource);
private:
    struct PlantInfo {
        double HelpOffer;
        double HelpFactor;
    };

    //
    //  This is the pool of plant that are connected to the mycorrhiza
    std::map<Plant*, PlantInfo> GoodPool;
    //
    //  This is the pool of plants available to the mycorrhiza
    std::vector<Plant*>      BadPool;
    //
    //  This is a set of pointers to plants to verify pointers passed to this
    //  object.
    std::set<Plant*>         PtrPool;
    //
    //  This is the maximum offered resource.
    double MaxOffer;
    double FbMin;
    double FbMax;
    double FbRange;
    double FbOffset;
};

#endif // MYCORRHIZA_H
