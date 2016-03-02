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
    virtual unsigned int dim()
    {
        return 1;
    }

    virtual void prepareForSequence()
    {
        struct timeval duration = { 2, 0 };

        add(duration, BUDGET_MAXIMUM);
        add(duration, ADDITIONAL_TIMEOUT);
        
        work(duration);
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
