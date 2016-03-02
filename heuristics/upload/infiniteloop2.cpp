#include <mash/heuristic.h>

using namespace Mash;


class InfiniteLoopHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    InfiniteLoopHeuristic() {}
    virtual ~InfiniteLoopHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareforImage()
    {
        while (true)
        {
        }
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new InfiniteLoopHeuristic();
}
