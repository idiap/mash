#include <mash/heuristic.h>

using namespace Mash;


class NoDimensionHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    NoDimensionHeuristic() {}
    virtual ~NoDimensionHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 0;
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new NoDimensionHeuristic();
}
