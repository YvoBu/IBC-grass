
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
       double pEstab;
       int age;
       bool toBeRemoved;

       Seed(const Traits& t, Cell* cell, ITV_mode itv, double aSD);
       Seed(const Traits& t, Cell* cell, const double estab, ITV_mode itv, double aSD);

       Cell* getCell() { return cell; }

       inline static bool GetSeedRemove(const std::unique_ptr<Seed> & s) {
           return s->toBeRemoved;
       };
};

#endif
