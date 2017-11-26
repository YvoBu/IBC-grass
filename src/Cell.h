
#ifndef SRC_CELL_H_
#define SRC_CELL_H_

#include <vector>

#include "Traits.h"
#include "Plant.h"
#include "Seed.h"

class Cell
{

public:
    int x;
    int y;

    double AResConc;  // above-ground resource availability
    double BResConc;  // below-ground resource availability

    double aComp_weekly;
    double bComp_weekly;

    bool occupied;       // is the cell occupied by any plant?

    std::vector< std::weak_ptr<Plant> > AbovePlantList; // List of all plant individuals that cover the cell ABOVE ground
    std::vector< std::weak_ptr<Plant> > BelowPlantList; // List of all plant individuals that cover the cell BELOW ground

    std::vector< std::unique_ptr<Seed> > SeedBankList; // List of all (ungerminated) seeds in the cell
    std::vector< std::unique_ptr<Seed> > SeedlingList; // List of all freshly germinated seedlings in the cell

    std::map<std::string, int> PftNIndA; // Plants covering the cell aboveground
    std::map<std::string, int> PftNIndB; // Plants covering the cell belowground

    Cell(const unsigned int xx,
         const unsigned int yy);

    virtual ~Cell();

    void weeklyReset();
    void SetResource(double Ares, double Bres);
    double Germinate();
    void RemoveSeeds();

    /* competition function for size symmetric above-ground resource competition
     * function is overwritten if inherited class with different competitive
     * size-asymmetry of niche differentiation is used
     */
    virtual void AboveComp() = 0;

    /* competition function for size symmetric below-ground resource competition
     * function is overwritten if inherited class with different competitive
     * size-asymmetry of niche differentiation is used
     */
    virtual void BelowComp() = 0;
};

class CellAsymPartSymV1 : public Cell {
public:
    CellAsymPartSymV1(const unsigned int xx,
                      const unsigned int yy) : Cell(xx,yy) {};
    virtual ~CellAsymPartSymV1() {};
    virtual void AboveComp();
    virtual void BelowComp();
};

class CellAsymPartSymV2 : public Cell {
public:
    CellAsymPartSymV2(const unsigned int xx,
                      const unsigned int yy) : Cell(xx,yy) {};
    virtual ~CellAsymPartSymV2() {};
    virtual void AboveComp();
    virtual void BelowComp();
private:
    virtual double prop_res_above(const std::string & type);
    virtual double prop_res_below(const std::string& type);
};

class CellAsymPartSymV3 : public CellAsymPartSymV2 {
public:
    CellAsymPartSymV3(const unsigned int xx,
                      const unsigned int yy) : CellAsymPartSymV2(xx,yy) {};
    virtual ~CellAsymPartSymV3() {};
private:
    virtual double prop_res_above(const std::string & type);
    virtual double prop_res_below(const std::string& type);
};
//---------------------------------------------------------------------------
#endif
