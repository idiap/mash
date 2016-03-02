#include <mash/heuristic.h>
#include <unistd.h>

using namespace Mash;


class SlowButGoodHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    SlowButGoodHeuristic() {}
    virtual ~SlowButGoodHeuristic() {}


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim()
    {
        return 5;
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        sleep(3);
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new SlowButGoodHeuristic();
}
