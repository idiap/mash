#include "timeout_heuristic.h"

using namespace Mash;


class TestHeuristic: public TimeoutHeuristic
{
    //_____ Construction / Destruction __________
public:
    TestHeuristic()
    : full_processing(false)
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

    virtual void finishForImage()
    {
        if (full_processing)
        {
            struct timeval duration = { 1, 0 };

            add(duration, BUDGET_MAXIMUM);

            work(duration);
        }
        else
        {
            struct timeval duration = { 1, 0 };

            add(duration, BUDGET_INITIALIZATION);
            add(duration, BUDGET_PER_SEQUENCE);
            add(duration, BUDGET_PER_PIXEL, image->width() * image->height());

            work(duration);
        }
    }

    virtual scalar_t computeFeature(unsigned int feature_index)
    {
        full_processing = true;
        
        return 0.0f;
    }


    //_____ Attributes __________
public:
    bool full_processing;
};


extern "C" Heuristic* new_heuristic()
{
    return new TestHeuristic();
}
