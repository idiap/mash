#include <mash/heuristic.h>
#include <math.h>

using namespace Mash;


class NaNHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    NaNHeuristic()
    {
    }

    virtual ~NaNHeuristic()
    {
    }


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0 * log(0);
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new NaNHeuristic();
}
