#include <mash/heuristic.h>

using namespace Mash;


class NoConstructorHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    NoConstructorHeuristic() {}
    virtual ~NoConstructorHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};
