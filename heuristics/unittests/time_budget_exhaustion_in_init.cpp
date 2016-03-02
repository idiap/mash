#include "timeout_heuristic.h"

using namespace Mash;


class TestHeuristic: public TimeoutHeuristic
{
    //_____ Construction / Destruction __________
public:
    TestHeuristic()
    {
    }

    virtual ~TestHeuristic()
    {
    }


    //_____ Implementation of Heuristic __________
public:
    virtual void init()
    {
        struct timeval duration = { 1, 0 };

        add(duration, BUDGET_INITIALIZATION);
        
        work(duration);
    }

    virtual unsigned int dim()
    {
        return 1;
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        return 0.0f;
    }
};


extern "C" Heuristic* new_heuristic()
{
    return new TestHeuristic();
}
