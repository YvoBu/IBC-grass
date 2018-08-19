
#ifndef SRC_SEED_H_
#define SRC_SEED_H_

#include <memory>

class Cell;
class Plant;

class Seed : public Traits
{
    protected:
       Cell* cell;

    public:
       double mass;
       double pSeedEstab;
       int age;

       Seed(const Traits& t, Cell* cell, ITV_mode itv, double aSD);
       Seed(const Traits& t, Cell* cell, const double estab, ITV_mode itv, double aSD);

       Cell* getCell() const { return cell; }
       bool OldAge() { if (age >= dormancy) { return true; } else {return false;};};

};

#endif
